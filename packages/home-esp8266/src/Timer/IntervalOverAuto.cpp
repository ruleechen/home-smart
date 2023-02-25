#include "IntervalOverAuto.h"

namespace Victor::Components {

  IntervalOverAuto::IntervalOverAuto(unsigned long interval) : IntervalOver(interval) {
    start();
  }

  bool IntervalOverAuto::isOver(unsigned long now) {
    const auto over = IntervalOver::isOver(now);
    // auto start when over
    if (over) { start(now); }
    // ret
    return over;
  }

} // namespace Victor::Components
