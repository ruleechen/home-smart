#ifndef OutletInUse_h
#define OutletInUse_h

#include <Arduino.h>
#include <Pin/DigitalInput.h>
#include <Timer/IntervalOver.h>

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
    IntervalOver* _debounce = nullptr;
  };

} // namespace Victor::Components

#endif // OutletInUse_h
