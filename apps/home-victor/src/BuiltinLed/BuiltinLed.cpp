#include "BuiltinLed.h"
#include "Storage/PinStorage/PinStorage.h"

namespace Victor::Components {
namespace {
    BuiltinLed* g_twinkleLed = nullptr;
    void twinkleCallback() {
        if (g_twinkleLed != nullptr) {
            g_twinkleLed->toggle();
        }
    }
} // namespace

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
    delete storage;
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
    g_twinkleLed = this;
    _ticker->attach_ms(duration, twinkleCallback);
  }

  void BuiltinLed::stop() {
    _ticker->detach();
    turnOff();
  }

  // global
  BuiltinLed builtinLed;

} // namespace Victor::Components
