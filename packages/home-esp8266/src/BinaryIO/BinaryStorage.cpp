#include "BinaryStorage.h"

namespace Victor::Components {

  BinaryStorage::BinaryStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void BinaryStorage::_serializeTo(const BinaryConfig* model, DynamicJsonDocument& doc) {
    // input pin
    const JsonArray inputArr = doc.createNestedArray(F("input"));
    inputArr[0] = model->input->pin;
    inputArr[1] = model->input->trueValue;
    // output pin
    const JsonArray outputArr = doc.createNestedArray(F("output"));
    outputArr[0] = model->output->pin;
    outputArr[1] = model->output->trueValue;
    // output2 pin
    const JsonArray output2Arr = doc.createNestedArray(F("output2"));
    output2Arr[0] = model->output2->pin;
    output2Arr[1] = model->output2->trueValue;
    // output state
    const JsonArray stateArr = doc.createNestedArray(F("state"));
    stateArr[0] = model->saveOutput ? 1 : 0;
    stateArr[1] = model->outputIsOn ? 1 : 0;
  }

  void BinaryStorage::_deserializeFrom(BinaryConfig* model, const DynamicJsonDocument& doc) {
    // input pin
    const auto inputArr = doc[F("input")];
    model->input = new PinConfig({
      .pin = inputArr[0],
      .trueValue = inputArr[1],
    });
    // output pin
    const auto outputArr = doc[F("output")];
    model->output = new PinConfig({
      .pin = outputArr[0],
      .trueValue = outputArr[1],
    });
    // output2 pin
    const auto output2Arr = doc[F("output2")];
    model->output2 = new PinConfig({
      .pin = output2Arr[0],
      .trueValue = output2Arr[1],
    });
    // output state
    const auto stateArr = doc[F("state")];
    model->saveOutput = stateArr[0] == 1;
    model->outputIsOn = stateArr[1] == 1;
  }

} // namespace Victor::Components
