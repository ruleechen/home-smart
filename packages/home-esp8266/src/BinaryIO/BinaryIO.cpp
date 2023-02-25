#include "BinaryIO.h"

namespace Victor::Components {

  BinaryIO::BinaryIO(const char* settingFile) {
    _storage = new BinaryStorage(settingFile);
    const auto setting = _storage->load();
    if (setting->inputPin > -1) {
      _input = new ActionButtonInput(setting->inputPin, setting->inputTrueValue);
      _input->onAction = [&](ButtonAction action) {
        if (onInputAction != nullptr) {
          onInputAction(action);
        }
      };
    }
    if (setting->outputPin > -1) {
      _output = new DigitalOutput(setting->outputPin, setting->outputTrueValue);
    }
    if (setting->output2Pin > -1) {
      _output2 = new DigitalOutput(setting->output2Pin, setting->output2TrueValue);
    }
    if (setting->saveOutput) {
      setOutputState(setting->outputIsOn);
    }
  }

  BinaryIO::~BinaryIO() {
    if (_storage != nullptr) {
      free(_storage);
      _storage = nullptr;
    }
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    if (_output != nullptr) {
      delete _output;
      _output = nullptr;
    }
    if (_output2 != nullptr) {
      delete _output2;
      _output2 = nullptr;
    }
  }

  void BinaryIO::loop() {
    if (_input != nullptr) {
      _input->loop();
    }
  }

  bool BinaryIO::getOutputState() {
    return _output != nullptr
      ? _output->lastValue()
      : false;
  }

  void BinaryIO::setOutputState(bool value) {
    if (_output != nullptr) {
      _output->setValue(value);
    }
    if (_output2 != nullptr) {
      _output2->setValue(value);
    }
    // save output state
    auto setting = _storage->load();
    if (
      setting->saveOutput &&
      setting->outputIsOn != value
    ) {
      setting->outputIsOn = value;
      _storage->save(setting);
    }
  }

} // namespace Victor::Components
