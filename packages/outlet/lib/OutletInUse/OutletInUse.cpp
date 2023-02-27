#include "OutletInUse.h"

namespace Victor::Components {

  OutletInUse::OutletInUse() {
    const auto irSendJson = new PinStorage("/irSend.json");
    const auto irRecvJson = new PinStorage("/irRecv.json");
    const auto sendPin = irSendJson->load();
    const auto recvPin = irRecvJson->load();
    if (sendPin->enable) {
      _irSend = new IRsend(sendPin->pin);
      _irSend->begin();
    }
    if (recvPin->enable) {
      _irRecv = new IRrecv(recvPin->pin);
      _irRecv->enableIRIn();
    }
  }

  OutletInUse::~OutletInUse() {
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onStateChange = nullptr;
  }

  bool OutletInUse::getState() {
    return true;
  }

  void OutletInUse::loop() {
    decode_results results;
    if (_irRecv->decode(&results)) {
      console.log(String(results.command));
      _irRecv->resume();
    }
  }

} // namespace Victor::Components
