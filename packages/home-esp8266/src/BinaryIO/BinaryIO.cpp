#include "BinaryIO.h"

namespace Victor::Components {

  BinaryIO::BinaryIO(const char* settingFile) {
    _storage = new BinaryStorage(settingFile);
    const auto setting = _storage->load();
    if (setting->input != nullptr && setting->input->enable) {
      _input = new ActionButtonInput(setting->input);
      _input->onAction = [&](ButtonAction action) {
        if (onInputAction != nullptr) {
          onInputAction(action);
        }
      };
    }
    if (setting->output != nullptr && setting->output->enable) {
      _output = new DigitalOutput(setting->output);
    }
    if (setting->output2 != nullptr && setting->output2->enable) {
      _output2 = new DigitalOutput(setting->output2);
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
