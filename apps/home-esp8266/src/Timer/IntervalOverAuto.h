#ifndef IntervalOverAuto_h
#define IntervalOverAuto_h

#include "Timer/IntervalOver.h"

namespace Victor::Components {
  class IntervalOverAuto : public IntervalOver {
   public:
    IntervalOverAuto(unsigned long interval);
    ~IntervalOverAuto();
    bool isOver(unsigned long now = 0) override;
  };
} // namespace Victor::Components

#endif // IntervalOverAuto_h
