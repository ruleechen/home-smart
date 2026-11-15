#include "OppleFE6300State.h"

namespace {
  const char* const POWER_OFF = "powerOff";
} // namespace

bool OppleFE6300State::apply(const String& command) {
  if (command == POWER_OFF) {
    _clearModes();
    return true;
  }
  const auto function = _find(command);
  if (function == nullptr) {
    return false;
  }
  const bool on = !function->on;
  if (function->mode) {
    _clearModes();
  }
  function->on = on;
  return true;
}

void OppleFE6300State::_clearModes() {
  for (auto& function : _functions) {
    if (function.mode) {
      function.on = false;
    }
  }
}

bool OppleFE6300State::isOn(const String& function) const {
  const auto found = _find(function);
  return found != nullptr && found->on;
}

String OppleFE6300State::describe() const {
  String text;
  for (const auto& function : _functions) {
    if (text.length() > 0) {
      text += ' ';
    }
    text += function.name;
    text += function.on ? F(":on") : F(":off");
  }
  return text;
}

OppleFE6300State::Function* OppleFE6300State::_find(const String& name) {
  for (auto& function : _functions) {
    if (name == function.name) {
      return &function;
    }
  }
  return nullptr;
}

const OppleFE6300State::Function* OppleFE6300State::_find(const String& name) const {
  for (const auto& function : _functions) {
    if (name == function.name) {
      return &function;
    }
  }
  return nullptr;
}
