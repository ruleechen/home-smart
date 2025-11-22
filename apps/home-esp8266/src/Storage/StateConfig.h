#ifndef StateConfig_h
#define StateConfig_h

namespace Victor::Components {

  template<class TState>
  struct StateConfig {
    // save state or not
    bool save = false;

    // default value
    TState* defaultValue;

    // state value saved
    TState* currentValue;
  };

} // namespace Victor::Components

#endif // StateConfig_h
