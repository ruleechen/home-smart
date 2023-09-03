#ifndef VictorRadio_h
#define VictorRadio_h

#include <Arduino.h>
#include "GlobalHelpers.h"
#include "Console.h"
#include "VictorOTA.h"
#include "VictorWifi.h"
#include "Storage/AppStorage/AppStorage.h"
#include "Storage/RadioStorage/RadioStorage.h"

#ifndef VICTOR_RADIO_DOUBLE_PRESS_MIN
#define VICTOR_RADIO_DOUBLE_PRESS_MIN 400
#endif

#ifndef VICTOR_RADIO_DOUBLE_PRESS_MAX
#define VICTOR_RADIO_DOUBLE_PRESS_MAX 800
#endif

#ifndef VICTOR_RADIO_LONG_PRESS_THRESHOLD
#define VICTOR_RADIO_LONG_PRESS_THRESHOLD 2000
#endif

#ifndef VICTOR_RADIO_RESET_THRESHOLD
#define VICTOR_RADIO_RESET_THRESHOLD 2500
#endif

namespace Victor::Components {
  class VictorRadio {
   public:
    VictorRadio();
    ~VictorRadio();
    void emit(const String& name);
    void emit(uint8_t index);
    typedef std::function<void(const RadioEmit* emit)> TRadioEmit;
    TRadioEmit onEmit = nullptr;
    void receive(const String& value, const uint8_t channel);
    typedef std::function<bool(const RadioRule* rule)> TRadioAction;
    TRadioAction onAction = nullptr;
    typedef std::function<bool(const RadioCommandParsed* command)> TRadioCommand;
    TRadioCommand onCommand = nullptr;

   private:
    RadioPressState _lastPressState = PRESS_STATE_AWAIT;
    void _handleEmit(const RadioEmit* emit);
    void _fireOnEmit(const RadioEmit* emit, const uint32_t nextDelay);
    void _handleReceived(const RadioMessage* message, const RadioPressState press);
    void _proceedAction(const RadioRule* rule);
    void _proceedCommand(const RadioCommandParsed* command);
    static RadioMessage* _parseMessage(const String& value, const uint8_t channel);
    static RadioCommandParsed* _parseCommand(const String& value);
  };
} // namespace Victor::Components

#endif // VictorRadio_h
