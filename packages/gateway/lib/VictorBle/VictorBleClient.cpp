#include "VictorBleClient.h"

namespace Victor::Components {

  VictorBleClient::VictorBleClient(BLEAdvertisedDevice* advertisedDevice) {
    serverDevice = advertisedDevice;
  }

  VictorBleClient::~VictorBleClient() {
    if (_client != nullptr) {
      delete _client;
      _client = nullptr;
    }
    if (_remoteCharacteristicReadable != nullptr) {
      delete _remoteCharacteristicReadable;
      _remoteCharacteristicReadable = nullptr;
    }
    if (_remoteCharacteristicWritable != nullptr) {
      delete _remoteCharacteristicWritable;
      _remoteCharacteristicWritable = nullptr;
    }
    if (_remoteCharacteristicNotifiable != nullptr) {
      delete _remoteCharacteristicNotifiable;
      _remoteCharacteristicNotifiable = nullptr;
    }
    if (onNotify != nullptr) {
      onNotify = nullptr;
    }
  }

  void VictorBleClient::loop() {
    if (_serverModel != RS_SERVER_MODEL_UNKNOWN) {
      const auto now = millis();
      const auto interval = VICTOR_BLE_HEARTBEAT_INTERVAL;
      if (interval > 0 && now - _lastHeartbeat > interval) {
        _lastHeartbeat = now;
        cmdHeartbeat();
      }
    }
  }

  bool VictorBleClient::connectServer() {
    _client = BLEDevice::createClient();
    const auto callbacks = new VictorBleClientCallbacks();
    callbacks->onConnectivityChange = [&](BLEClient* client, bool connected) {
      if (onConnectivityChange != nullptr) {
        onConnectivityChange(this, connected);
      }
    };
    _client->setClientCallbacks(callbacks);

    const auto success = _client->connect(serverDevice);
    if (success) {
      const auto remoteServices = _client->getServices();
      for (auto service = remoteServices->begin(); service != remoteServices->end(); service++) {
        const auto remoteCharacteristics = service->second->getCharacteristics();
        for (auto characteristic = remoteCharacteristics->begin(); characteristic != remoteCharacteristics->end(); characteristic++) {
          const auto remoteCharacteristic = characteristic->second;
          if (remoteCharacteristic->canRead() && _remoteCharacteristicReadable == nullptr) {
            _remoteCharacteristicReadable = remoteCharacteristic;
            if (remoteCharacteristic->canNotify() && _remoteCharacteristicNotifiable == nullptr) {
              _remoteCharacteristicNotifiable = remoteCharacteristic;
            }
          }
          if (remoteCharacteristic->canWrite() && _remoteCharacteristicWritable == nullptr) {
            _remoteCharacteristicWritable = remoteCharacteristic;
          }
        }
      }
    }

    if (_remoteCharacteristicNotifiable != nullptr) {
      _remoteCharacteristicNotifiable->registerForNotify([&](BLERemoteCharacteristic* remoteCharacteristic, uint8_t* payload, size_t length, bool isNotify) {
        #ifdef VICTOR_DEBUG
          auto serverAddress = serverDevice->getAddress().toString().c_str();
          Serial.printf("[%s] notifying: '%s'", serverAddress, payload); Serial.println();
        #endif
        const auto notification = deserializeServerCommand(payload, length);
        if (
          notification->command == RS_SERVER_COMMAND_AUTHENTICATE ||
          notification->command == RS_SERVER_COMMAND_HEARTBEAT ||
          notification->command == RS_SERVER_COMMAND_GET_STATES ||
          notification->command == RS_SERVER_COMMAND_SET_STATES ||
          notification->command == RS_SERVER_COMMAND_NOTIFY_STATES
        ) {
          _stateBattery = ((uint16_t)notification->data1 << 8) | notification->data2;
          _stateOTA = notification->data3 == RS_SERVER_STATE_OTA_ON;
          _stateInput = notification->data4 == RS_SERVER_STATE_INPUT_ON;
          _stateOutput = notification->data5 == RS_SERVER_STATE_OUTPUT_ON;
        }
        if (notification->command == RS_SERVER_COMMAND_AUTHENTICATE) {
          _lastHeartbeat = millis();
          _serverModel = (RsServerModelType)notification->model;
        } else if (notification->command == RS_SERVER_COMMAND_HEARTBEAT) {
          _lastHeartbeat = millis();
        }
        if (onNotify != nullptr) {
          onNotify(this, notification);
        }
      });
      cmdAuthenticate();
    }

    return success;
  }

  bool VictorBleClient::isConnected() {
    return _client->isConnected();
  }

  bool VictorBleClient::send(uint8_t* payload, size_t length) {
    auto sent = false;
    if (_client->isConnected() && _remoteCharacteristicWritable != nullptr) {
      #ifdef VICTOR_DEBUG
        auto serverAddress = serverDevice->getAddress().toString().c_str();
        Serial.printf("[%s] writing: '%s'", serverAddress, payload); Serial.println();
      #endif
      _remoteCharacteristicWritable->writeValue(payload, length, false);
      sent = true;
    }
    return sent;
  }

  bool VictorBleClient::send(ServerCommand* command) {
    uint8_t payload[RS_PROTOCOL_BYTES] = {0};
    serializeServerCommand(payload, command);
    const auto sent = send(payload, RS_PROTOCOL_BYTES);
    return sent;
  }

  void VictorBleClient::cmdAuthenticate() {
    auto cmd = new ServerCommand();
    cmd->model   = _serverModel;
    cmd->command = RS_SERVER_COMMAND_AUTHENTICATE;
    cmd->data1   = RS_PROTOCOL_AUTH_TOKEN1;
    cmd->data2   = RS_PROTOCOL_AUTH_TOKEN2;
    cmd->data3   = RS_PROTOCOL_AUTH_TOKEN3;
    cmd->data4   = RS_PROTOCOL_AUTH_TOKEN4;
    send(cmd);
    delete cmd;
  }

  void VictorBleClient::cmdHeartbeat() {
    auto cmd = new ServerCommand();
    cmd->model   = _serverModel;
    cmd->command = RS_SERVER_COMMAND_HEARTBEAT;
    send(cmd);
    delete cmd;
  }

  void VictorBleClient::cmdReset() {
    auto cmd = new ServerCommand();
    cmd->model   = _serverModel;
    cmd->command = RS_SERVER_COMMAND_RESET;
    send(cmd);
    delete cmd;
  }

  void VictorBleClient::cmdGetStates() {
    auto cmd = new ServerCommand();
    cmd->model   = _serverModel;
    cmd->command = RS_SERVER_COMMAND_GET_STATES;
    send(cmd);
    delete cmd;
  }

  void VictorBleClient::cmdSetStates(bool outputOn, bool otaOn) {
    auto cmd = new ServerCommand();
    cmd->model   = _serverModel;
    cmd->command = RS_SERVER_COMMAND_SET_STATES;
    cmd->data3   = otaOn    ? RS_SERVER_STATE_OTA_ON    : RS_SERVER_STATE_UNKNOWN;
    cmd->data5   = outputOn ? RS_SERVER_STATE_OUTPUT_ON : RS_SERVER_STATE_UNKNOWN;
    send(cmd);
    delete cmd;
  }

  uint16_t VictorBleClient::stateBattery() {
    return _stateBattery;
  }

  bool VictorBleClient::stateOTA() {
    return _stateOTA;
  }

  bool VictorBleClient::stateInput() {
    return _stateInput;
  }

  bool VictorBleClient::stateOutput() {
    return _stateOutput;
  }

  void VictorBleClient::serializeServerCommand(uint8_t* payload, ServerCommand* command) {
    rsFillMessage(payload,
      command->model,
      command->command,
      command->data1,
      command->data2,
      command->data3,
      command->data4,
      command->data5,
      command->data6,
      command->data7,
      command->data8
    );
  }

  ServerCommand* VictorBleClient::deserializeServerCommand(uint8_t* payload, size_t length) {
    bool valid = rsValidateMessage(payload, length);
    if (valid == false) {
      return nullptr;
    }
    auto command = new ServerCommand();
    command->model   = payload[RS_PROTOCOL_INDEX_MODEL];
    command->command = payload[RS_PROTOCOL_INDEX_COMMAND];
    command->data1   = payload[RS_PROTOCOL_INDEX_DATA1];
    command->data2   = payload[RS_PROTOCOL_INDEX_DATA2];
    command->data3   = payload[RS_PROTOCOL_INDEX_DATA3];
    command->data4   = payload[RS_PROTOCOL_INDEX_DATA4];
    command->data5   = payload[RS_PROTOCOL_INDEX_DATA5];
    command->data6   = payload[RS_PROTOCOL_INDEX_DATA6];
    command->data7   = payload[RS_PROTOCOL_INDEX_DATA7];
    command->data8   = payload[RS_PROTOCOL_INDEX_DATA8];
    return command;
  }

} // namespace Victor::Components
