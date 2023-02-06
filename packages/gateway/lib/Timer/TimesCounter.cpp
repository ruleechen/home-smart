#include "TimesCounter.h"

namespace Victor::Components {

  TimesCounter::TimesCounter(unsigned long resetMillis) {
    _reset = new IntervalOver(resetMillis);
  }

  TimesCounter::~TimesCounter() {
    onCount = nullptr;
    if (_reset != nullptr) {
      free(_reset); // delete _reset;
      _reset = nullptr;
    }
  }

  void TimesCounter::count() {
    const auto now = millis();
    if (_reset->isOver(now)) {
      reset();
    }
    _reset->start(now);
    _count++;
    if (onCount != nullptr) {
      onCount(_count);
    }
  }

  void TimesCounter::reset() {
    _count = 0;
  }

} // namespace Victor::Components
