#ifndef StateSensor_h
#define StateSensor_h

#include <Arduino.h>
#include <Timer/IntervalOver.h>
#include <Timer/IntervalOverAuto.h>

namespace Victor::Components {

  enum StateSensorChange {
    STATE_CHANGE_NO   = 0,
    STATE_CHANGE_YES  = 1,
    STATE_CHANGE_NONE = 2,
  };

  template <typename TState>
  class StateSensor {
   public:
    StateSensor(const TState defaultState);
    ~StateSensor();
    void setDebounce(const uint16_t ms);
    void setHeartbeat(const uint16_t ms);
    static void reportChange();
    virtual TState readState();
    void loop();
    // events
    typedef std::function<void(const TState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;
    typedef std::function<void(const TState state)> THeartbeatHandler;
    THeartbeatHandler onHeartbeat = nullptr;

   protected:
    TState _state;
    IntervalOver* _debounce = nullptr;
    IntervalOverAuto* _heartbeat = nullptr;
    // interrupt
    volatile static StateSensorChange _changed;

   private:
    void _cancelDebounce();
    void _cancelHeartbeat();
  };

  template <typename TState>
  StateSensor<TState>::StateSensor(const TState defaultState) {
    _state = defaultState;
  }

  template <typename TState>
  StateSensor<TState>::~StateSensor() {
    _cancelDebounce();
    _cancelHeartbeat();
    onStateChange = nullptr;
    onHeartbeat = nullptr;
  }

  template <typename TState>
  void StateSensor<TState>::setDebounce(const uint16_t ms) {
    _cancelDebounce();
    if (ms > 0) {
      _debounce = new IntervalOver(ms);
    }
  }

  template <typename TState>
  void StateSensor<TState>::setHeartbeat(const uint16_t ms) {
    _cancelHeartbeat();
    if (ms > 0) {
      _heartbeat = new IntervalOverAuto(ms);
    }
  }

  template <typename TState>
  void StateSensor<TState>::_cancelDebounce() {
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
  }

  template <typename TState>
  void StateSensor<TState>::_cancelHeartbeat() {
    if (_heartbeat != nullptr) {
      free(_heartbeat);
      _heartbeat = nullptr;
    }
  }

  template <typename TState>
  void StateSensor<TState>::loop() {
    const auto now = millis();
    if (
      _changed == STATE_CHANGE_YES ||
      _changed == STATE_CHANGE_NONE
    ) {
      if (
        _debounce == nullptr ||
        _debounce->isOver(now)
      ) {
        if (_changed == STATE_CHANGE_YES) {
          _changed = STATE_CHANGE_NO;
        }
        if (_debounce != nullptr) {
          _debounce->start(now);
        }
        const auto state = readState();
        if (state != _state) {
          _state = state;
          if (onStateChange != nullptr) {
            onStateChange(_state);
          }
        }
      }
    }
    if (
      _heartbeat != nullptr &&
      _heartbeat->isOver(now)
    ) {
      if (onHeartbeat != nullptr) {
        onHeartbeat(_state);
      }
    }
  }

  template <typename TState>
  volatile StateSensorChange StateSensor<TState>::_changed = STATE_CHANGE_NONE;

  template <typename TState>
  void StateSensor<TState>::reportChange() {
    _changed = STATE_CHANGE_YES;
  }

} // namespace Victor::Components

#endif // StateSensor_h
