#ifndef Mesher_h
#define Mesher_h

#include <functional>
#include <Arduino.h>
#include "Commons.h"

namespace Victor::Components {

  enum MeshMessageType {
    MESH_HEARTBEAT = 0,
    MESH_WIFI_REQUEST = 1,
    MESH_WIFI_CONNECTED = 2,
  };

  struct MeshMessage {
    MeshMessageType type;
    String sourceId;
    String replyId;
    String content;
  };

  class IMeshLoader {
    typedef std::function<void(MeshMessage&)> TMessageHandler;

   public:
    IMeshLoader(uint8_t inputPin);
    void onMessage(TMessageHandler handler);
    virtual void send(const MeshMessage& message);

   protected:
    uint8_t _inputPin;
    TMessageHandler _messageHandler;
  };

  class RadioFrequencyMeshLoader : public IMeshLoader {
   public:
    RadioFrequencyMeshLoader(uint8_t inputPin);
    virtual void send(const MeshMessage& message);
  };

  class BluetoothMeshLoader : public IMeshLoader {
   public:
    BluetoothMeshLoader(uint8_t inputPin);
    virtual void send(const MeshMessage& message);
  };

  class Mesher {
   public:
    Mesher();
    void setLoader(IMeshLoader* loader);
    void send(MeshMessageType type);
    void send(MeshMessageType type, const String& content);

   private:
    String _id;
    IMeshLoader* _loader;
    void _handleMessage(MeshMessage& message);
  };
} // namespace Victor::Components

#endif // Mesher_h
