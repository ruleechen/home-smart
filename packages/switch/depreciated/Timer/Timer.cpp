#include <vector>
#include "Timer.h"

namespace Victor::Events {

  Timer::Timer() {}

  unsigned int Timer::setTimeout(unsigned long timespan, TCallback callback) {
    return _addConfig(false, timespan, callback);
  }

  unsigned int Timer::setInterval(unsigned long timespan, TCallback callback) {
    return _addConfig(true, timespan, callback);
  }

  bool Timer::clearTimeout(unsigned int id) {
    return _removeConfig(id);
  }

  bool Timer::clearInterval(unsigned int id) {
    return _removeConfig(id);
  }

  void Timer::loop() {
    std::vector<unsigned int> hitIds;
    const auto now = millis();
    for (const auto& item : _configs) {
      const Config config = item.second;
      if (now - config.timestamp > config.timespan) {
        hitIds.push_back(item.first);
      }
    }
    for (const auto& id : hitIds) {
      Config& config = _configs[id];
      if (config.repeat) {
        config.timestamp = now;
      } else {
        _removeConfig(id);
      }
      if (config.callback) {
        config.callback();
      }
    }
  }

  unsigned int Timer::_addConfig(bool repeat, unsigned long timespan, TCallback callback) {
    const Config config = {
      .repeat = repeat,
      .timespan = timespan,
      .callback = callback,
      .timestamp = millis(),
    };
    _idSeed = _idSeed + 1;
    _configs[_idSeed] = config;
    return _idSeed;
  }

  bool Timer::_removeConfig(unsigned int id) {
    if (_configs.count(id) > 0) {
      _configs.erase(id);
      return true;
    }
    return false;
  }

  // global
  Timer timer;

} // namespace Victor::Events
