#include "Mesher.h"

namespace Victor::Components {

  // IMeshLoader
  IMeshLoader::IMeshLoader(uint8_t inputPin) {
    _inputPin = inputPin;
  }
  void IMeshLoader::onMessage(TMessageHandler handler) {
    _messageHandler = handler;
  }
  void IMeshLoader::send(const MeshMessage& message) {
    console.log().bracket("IMeshLoader").section("send()"));
  }

  // RadioFrequencyMeshLoader
  RadioFrequencyMeshLoader::RadioFrequencyMeshLoader(uint8_t inputPin) : IMeshLoader(inputPin) {
  }
  void RadioFrequencyMeshLoader::send(const MeshMessage& message) {
    console.log().bracket("RadioFrequencyMeshLoader").section("send()"));
  }

  // BluetoothMeshLoader
  BluetoothMeshLoader::BluetoothMeshLoader(uint8_t inputPin) : IMeshLoader(inputPin) {
  }
  void BluetoothMeshLoader::send(const MeshMessage& message) {
    console.log().bracket("BluetoothMeshLoader").section("send()"));
  }

  // Mesher
  Mesher::Mesher() {
    _id = GlobalHelpers::randomString(8);
  }
  void Mesher::setLoader(IMeshLoader* loader) {
    _loader = loader;
    _loader->onMessage(std::bind(&Mesher::_handleMessage, this, std::placeholders::_1));
  }
  void Mesher::_handleMessage(MeshMessage& message) {
    if (message.type == MESH_HEARTBEAT) { // heartbeat
      if (message.sourceId == _id) {
        // ignore (launched by ourself)
      } else {
        message.replyId = _id;
        message.content = F("pong");
        _loader->send(message);
      }
    } else if (message.type == MESH_WIFI_REQUEST) {
      if (message.sourceId == _id) {
        // check wifi
        // connect wifi
      } else {
        // check wifi
        // reply wifi credential
      }
    } else if (message.type == MESH_WIFI_CONNECTED) {
      if (message.sourceId == _id) {
        // ignore (launched by ourself)
      } else {
        // check wifi
        // connect wifi
      }
    }
  }
  void Mesher::send(MeshMessageType type) {
    send(type, F(""));
  }
  void Mesher::send(MeshMessageType type, const String& content) {
    const MeshMessage message = {
      .type = type,
      .sourceId = _id,
      .replyId = F(""),
      .content = content,
    };
    _loader->send(message);
  }

} // namespace Victor::Components
