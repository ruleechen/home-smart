#ifndef StateSensor_h
#define StateSensor_h

#include <Arduino.h>
#include "Timer/IntervalOver.h"

namespace Victor::Components {

  enum StateSensorChange {
    STATE_CHANGE_NO   = 0,
    STATE_CHANGE_YES  = 1,
    STATE_CHANGE_NONE = 2,
  };

  template<typename TState>
  struct ReadStateEvent {
    TState state;
    bool cancel = false;
  };

  template <typename TState>
  class StateSensor {
   public:
    StateSensor(const TState defaultState);
    ~StateSensor();
    void setDebounce(const uint16_t ms);
    virtual TState readState();
    void loop();
    // events
    typedef std::function<void(const TState state)> TStateHandler;
    TStateHandler onStateChange = nullptr;
    TStateHandler onAfterReadState = nullptr;
    typedef std::function<void(ReadStateEvent<TState>* ev)> TReadStateHandler;
    TReadStateHandler onBeforeReadState = nullptr;

   protected:
    TState _state;
    IntervalOver* _debounce = nullptr;
    StateSensorChange _changed = STATE_CHANGE_NONE;

   private:
    void _cancelDebounce();
  };

  template <typename TState>
  StateSensor<TState>::StateSensor(const TState defaultState) {
    _state = defaultState;
  }

  template <typename TState>
  StateSensor<TState>::~StateSensor() {
    _cancelDebounce();
    onBeforeReadState = nullptr;
    onAfterReadState = nullptr;
    onStateChange = nullptr;
  }

  template <typename TState>
  void StateSensor<TState>::setDebounce(const uint16_t ms) {
    _cancelDebounce();
    if (ms > 0) {
      _debounce = new IntervalOver(ms);
    }
  }

  template <typename TState>
  void StateSensor<TState>::_cancelDebounce() {
    if (_debounce != nullptr) {
      delete _debounce;
      _debounce = nullptr;
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
        if (onBeforeReadState != nullptr) {
          const auto ev = new ReadStateEvent<TState>({
            .state = _state,
          });
          onBeforeReadState(ev);
          if (ev->cancel) {
            delete ev;
            return;
          }
          delete ev;
        }
        const auto state = readState();
        if (onAfterReadState != nullptr) {
          onAfterReadState(state);
        }
        if (state != _state) {
          _state = state;
          if (onStateChange != nullptr) {
            onStateChange(_state);
          }
        }
      }
    }
  }

} // namespace Victor::Components

#endif // StateSensor_h
