#ifndef VictorBleClient_h
#define VictorBleClient_h

#include <functional>
#include <Arduino.h>
#include <BLEDevice.h>
#include "VictorBleModels.h"

#ifndef VICTOR_BLE_HEARTBEAT_INTERVAL
#define VICTOR_BLE_HEARTBEAT_INTERVAL 6 * 60 * 60 * 1000
#endif

namespace Victor::Components {
  class VictorBleClientCallbacks : public BLEClientCallbacks {
   public:
    typedef std::function<void(BLEClient* client, bool connected)> TConnectivityHandler;
    TConnectivityHandler onConnectivityChange = nullptr;
    ~VictorBleClientCallbacks() {
      onConnectivityChange = nullptr;
    }
    void onConnect(BLEClient* client) {
      if (onConnectivityChange != nullptr) {
        onConnectivityChange(client, true);
      }
    }
    void onDisconnect(BLEClient* client) {
      if (onConnectivityChange != nullptr) {
        onConnectivityChange(client, false);
      }
    }
  };

  class VictorBleClient {
   public:
    VictorBleClient(BLEAdvertisedDevice* advertisedDevice);
    ~VictorBleClient();
    BLEAdvertisedDevice* serverDevice = nullptr;
    void loop();
    bool connectServer();
    bool isConnected();
    bool send(uint8_t* message, size_t length);
    bool send(ServerCommand* command);
    void cmdAuthenticate();
    void cmdHeartbeat();
    void cmdReset();
    void cmdGetStates();
    void cmdSetStates(bool outputOn, bool otaOn = true);
    uint16_t stateBattery();
    bool stateOTA();
    bool stateInput();
    bool stateOutput();
    static void serializeServerCommand(uint8_t* payload, ServerCommand* command);
    static ServerCommand* deserializeServerCommand(uint8_t* payload, size_t length);
    typedef std::function<void(VictorBleClient* client, ServerCommand* notification)> TNotifyHandler;
    TNotifyHandler onNotify = nullptr;
    typedef std::function<void(VictorBleClient* client, bool connected)> TConnectivityHandler;
    TConnectivityHandler onConnectivityChange = nullptr;

   private:
    BLEClient* _client = nullptr;
    unsigned long _lastHeartbeat = 0;
    RsServerModelType _serverModel = RS_SERVER_MODEL_UNKNOWN;
    uint16_t _stateBattery = 0;
    bool _stateOTA = false;
    bool _stateInput = false;
    bool _stateOutput = false;
    BLERemoteCharacteristic* _remoteCharacteristicReadable = nullptr;
    BLERemoteCharacteristic* _remoteCharacteristicWritable = nullptr;
    BLERemoteCharacteristic* _remoteCharacteristicNotifiable = nullptr;
  };

} // namespace Victor::Components

#endif // VictorBleClient_h
