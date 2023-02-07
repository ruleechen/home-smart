#include <map>
#include <vector>
#include <Arduino.h>
#include <BLEDevice.h>
#include "IntervalOverAuto.h"
#include "VictorBleClient.h"
#include "DoorSensor.h"

using namespace Victor;
using namespace Victor::Components;

DoorSensor* accessory = nullptr;

bool ledOn = false;
void turnLedOn() {
  #ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, HIGH);
  #endif
  ledOn = true;
}
void turnLedOff() {
  #ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, LOW);
  #endif
  ledOn = false;
}
void toggleLed() {
  if (ledOn) {
    turnLedOff();
  } else {
    turnLedOn();
  }
}
void flashLed(unsigned long ms = 100) {
  toggleLed();
  delay(ms); // at least light for some time
  toggleLed();
}

static std::vector<BLEAdvertisedDevice> devicesMatched = {};
static std::map<std::string, VictorBleClient*> clients = {};
// static BLEUUID advertisingServiceUUID("0000af00-0000-1000-8000-00805f9b34fb");

IntervalOverAuto* scanInterval;
IntervalOverAuto* cleanInterval;

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.printf("Scan [%s]", advertisedDevice.toString().c_str()); Serial.println();
    if (
      advertisedDevice.haveServiceUUID() &&
      advertisedDevice.getManufacturerData() == RS_PROTOCOL_MANUFACTURER_DATA
    ) {
      const auto address = advertisedDevice.getAddress().toString();
      if (clients.count(address) == 0) {
        clients[address] = nullptr;
        devicesMatched.push_back(advertisedDevice);
        Serial.printf(">> match [%s]", address.c_str()); Serial.println();
      }
    }
  }
};

static BLEScan* scan = nullptr;

void startScan() {
  Serial.println("Start scanning...");
  if (scan == nullptr) {
    scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    // scan->setInterval(1349);
    // scan->setWindow(449);
    scan->setActiveScan(true);
  } else {
    scan->clearResults();
  }
  scan->start(5, true); // scan for 5s (server broadcast every 2s)
  Serial.println("End scanning...");
}

void setup() {
  Serial.begin(115200);
  accessory = new DoorSensor();
  accessory->setup();

  #ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
  #endif
  turnLedOn();

  // ble init
  BLEDevice::init("Victor-Gateway");
  BLEDevice::setPower(ESP_PWR_LVL_P9); // max +9dbm
  // timers
  scanInterval = new IntervalOverAuto(5 * 60 * 1000); // 5 minutes
  cleanInterval = new IntervalOverAuto(5 * 1000); // 5 seconds
  // scan
  startScan();
  // done
  Serial.println();
  Serial.println("setup complete");
  turnLedOff();
}

void onConnectivityChange(VictorBleClient* client, bool connected) {
  auto serverAddress = client->serverDevice->getAddress().toString().c_str();
  Serial.printf("[%s] %s", serverAddress, connected ? "connected" : "disconnected"); Serial.println();
}

void onServerNotify(VictorBleClient* client, ServerCommand* notification) {
  auto serverAddress = client->serverDevice->getAddress().toString().c_str();
  auto battery = ((uint16_t)(notification->data1 << 8)) | notification->data2;
  auto ota    = client->stateOTA()    ? "ON" : "OFF";
  auto input  = client->stateInput()  ? "ON" : "OFF";
  auto output = client->stateOutput() ? "ON" : "OFF";
  Serial.printf("[%s] BTY:%04d", serverAddress, battery); Serial.println();
  Serial.printf("[%s] OTA:%s", serverAddress, ota); Serial.println();
  Serial.printf("[%s] INPUT:%s", serverAddress, input); Serial.println();
  Serial.printf("[%s] OUTPUT:%s", serverAddress, output); Serial.println();
  if (notification->command == RS_SERVER_COMMAND_NOTIFY_STATES) {
    if (client->stateInput()) {
      accessory->setOpen();
      turnLedOn();
    } else {
      accessory->setClosed();
      turnLedOff();
    }
  }
}

static String message = "";

void loop() {
  accessory->loop();

  const auto now = millis();
  if (scanInterval->isOver(now)) {
    startScan();
    return;
  }

  auto isEnterPressed = false;
  while (Serial.available()) {
    const auto ch = Serial.read();
    isEnterPressed = (ch == '\r');
    if (!isEnterPressed) {
      message += (char)ch;
    } else {
      break;
    }
  }

  if (isEnterPressed) {
    message.trim();
    if (message == "scan") {
      startScan();
    } else {
      for (auto it = clients.begin(); it != clients.end(); ++it) {
        const auto client = clients[it->first];
        if (message == "hb") {
          client->cmdHeartbeat();
        } else if (message == "get") {
          client->cmdGetStates();
        } else if (message == "set:0") {
          client->cmdSetStates(false);
        } else if (message == "set:1") {
          client->cmdSetStates(true);
        } else if (message == "reset") {
          client->cmdReset();
        } else if (message == "auth") {
          client->cmdAuthenticate();
        } else {
          // client->send((uint8_t*)message.c_str(), message.length());
        }
      }
    }
    flashLed();
    message = "";
    return;
  }

  if (!devicesMatched.empty()) {
    const auto firstMatch = devicesMatched[0];
    const auto device = new BLEAdvertisedDevice(firstMatch);
    const auto client = new VictorBleClient(device);
    client->onConnectivityChange = onConnectivityChange;
    client->onNotify = onServerNotify;
    const auto address = device->getAddress().toString();
    clients[address] = client;
    Serial.printf("[%s] connecting", address.c_str()); Serial.println();
    client->connectServer();
    devicesMatched.erase(devicesMatched.begin());
    return;
  }

  if (cleanInterval->isOver(now)) {
    std::vector<std::string> disconnectedAddresses = {};
    for (auto it = clients.begin(); it != clients.end(); ++it) {
      const auto client = clients[it->first];
      if (!client->isConnected()) {
        disconnectedAddresses.push_back(it->first);
      } else {
        client->loop();
      }
    }
    for (auto address : disconnectedAddresses) {
      clients.erase(address);
      Serial.printf("[%s] removed", address.c_str()); Serial.println();
    }
    if (clients.size() > 0) {
      flashLed();
    }
  }
}
