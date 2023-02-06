#include "IntervalOver.h"

namespace Victor::Components {

  IntervalOver::IntervalOver(unsigned long interval) {
    _interval = interval;
  }

  void IntervalOver::start(unsigned long now) {
    // https://www.arduino.cc/reference/en/language/functions/time/millis/
    // Returns the number of milliseconds passed since the Arduino board began running the current program.
    // This number will overflow (go back to zero), after approximately 50 days.
    if (now == 0) { now = millis(); }
    _start = now;
  }

  bool IntervalOver::isOver(unsigned long now) {
    if (now == 0) { now = millis(); }
    return (now - _start > _interval);
  }

} // namespace Victor::Components
