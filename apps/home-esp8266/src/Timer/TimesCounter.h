#ifndef TimesCounter_h
#define TimesCounter_h

#include <functional>
#include "Timer/IntervalOver.h"

namespace Victor::Components {
  class TimesCounter {
   public:
    TimesCounter(unsigned long resetMillis);
    ~TimesCounter();
    typedef std::function<void(uint8_t count)> TCountHandler;
    TCountHandler onCount = nullptr;
    void count();
    void reset();

   private:
    // args
    IntervalOver* _reset = nullptr;
    // state
    uint8_t _count = 0;
  };
} // namespace Victor::Components

#endif // TimesCounter_h
