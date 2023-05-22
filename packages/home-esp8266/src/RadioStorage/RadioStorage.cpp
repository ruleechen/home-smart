#include "RadioStorage.h"

namespace Victor::Components {

  RadioStorage::RadioStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 4096;
  }

  void RadioStorage::broadcast(RadioMessage* message) {
    if (_lastReceived != nullptr) {
      delete _lastReceived;
      _lastReceived = nullptr;
    }
    _lastReceived = message;
  }

  RadioMessage* RadioStorage::getLastReceived() const {
    return _lastReceived;
  }

  void RadioStorage::_serialize(const RadioModel* model, DynamicJsonDocument& doc) {
    doc[F("input")] = model->inputPin;
    doc[F("output")] = model->outputPin;
    const JsonArray emitItems = doc.createNestedArray(F("emits"));
    for (const auto emit : model->emits) {
      const JsonArray item = emitItems.createNestedArray();
      item[0] = emit->name;
      item[1] = emit->value;
      item[2] = emit->channel;
      item[3] = emit->press;
    }
    const JsonArray ruleItems = doc.createNestedArray(F("rules"));
    for (const auto rule : model->rules) {
      const JsonArray item = ruleItems.createNestedArray();
      item[0] = rule->value;
      item[1] = rule->channel;
      item[2] = rule->press;
      item[3] = rule->action;
    }
    const JsonArray commandItems = doc.createNestedArray(F("commands"));
    for (const auto command : model->commands) {
      const JsonArray item = commandItems.createNestedArray();
      item[0] = command->entry;
      item[1] = command->action;
      item[2] = command->press;
    }
  }

  void RadioStorage::_deserialize(RadioModel* model, const DynamicJsonDocument& doc) {
    model->inputPin = doc[F("input")];
    model->outputPin = doc[F("output")];
    const auto emitItems = doc[F("emits")];
    for (size_t i = 0; i < emitItems.size(); i++) {
      const auto item = emitItems[i];
      const auto emit = new RadioEmit({
        .name    = item[0],
        .value   = item[1],
        .channel = item[2],
        .press   = item[3],
      });
      model->emits.push_back(emit);
    }
    auto ruleItems = doc[F("rules")];
    for (size_t i = 0; i < ruleItems.size(); i++) {
      const auto item = ruleItems[i];
      const auto rule = new RadioRule({
        .value   = item[0],
        .channel = item[1],
        .press   = item[2],
        .action  = item[3],
      });
      model->rules.push_back(rule);
    }
    auto commandItems = doc[F("commands")];
    for (size_t i = 0; i < commandItems.size(); i++) {
      const auto item = commandItems[i];
      const auto command = new RadioCommand({
        .entry  = item[0],
        .action = item[1],
        .press  = item[2],
      });
      model->commands.push_back(command);
    }
  }

  // global
  RadioStorage radioStorage;

} // namespace Victor::Components
