#include "BuiltinLed.h"

namespace Victor::Components {

  BuiltinLed::BuiltinLed() {
    _ticker = new Ticker();
  }

  BuiltinLed::~BuiltinLed() {
    if (_ticker != nullptr) {
      delete _ticker;
      _ticker = nullptr;
    }
    if (_outputPin != nullptr) {
      delete _outputPin;
      _outputPin = nullptr;
    }
  }

  void BuiltinLed::setup(const char* configFile) {
    const auto storage = new PinStorage(configFile);
    const auto config = storage->load();
    if (config != nullptr && config->enable) {
      _outputPin = new DigitalOutput(config);
    }
  }

  void BuiltinLed::turnOn() {
    if (_outputPin != nullptr) {
      _outputPin->setValue(true);
    }
  }

  void BuiltinLed::turnOff() {
    if (_outputPin != nullptr) {
      _outputPin->setValue(false);
    }
  }

  void BuiltinLed::toggle() {
    if (_outputPin != nullptr) {
      const auto value = _outputPin->lastValue();
      _outputPin->setValue(!value);
    }
  }

  void BuiltinLed::flash(uint16_t duration) {
    toggle();
    delay(duration);
    toggle();
  }

  void BuiltinLed::twinkle(uint16_t duration) {
    toggle();
    _ticker->detach();
    _ticker->attach_ms(duration, [&]() {
      toggle();
    });
  }

  void BuiltinLed::stop() {
    _ticker->detach();
    turnOff();
  }

  // global
  BuiltinLed builtinLed;

} // namespace Victor::Components
