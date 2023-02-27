#ifndef OutletInUse_h
#define OutletInUse_h

#include <Arduino.h>
#include <Console.h>
#include <Pin/PinStorage.h>
#include <Timer/IntervalOver.h>

#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

namespace Victor::Components {
  class OutletInUse {
   public:
    OutletInUse();
    ~OutletInUse();
    bool getState();
    void loop();
    // events
    typedef std::function<void(const bool state)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    IRsend* _irSend = nullptr;
    IRrecv* _irRecv = nullptr;
    IntervalOver* _debounce = nullptr;
  };

} // namespace Victor::Components

#endif // OutletInUse_h
