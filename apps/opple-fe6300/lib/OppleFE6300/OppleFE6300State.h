#ifndef OppleFE6300State_h
#define OppleFE6300State_h

#include <Arduino.h>

// Heater state inferred from key presses: the heater never reports it.
// Verified on the heater:
//   - the light is on its own: its key toggles it, nothing else touches it;
//   - heat, blow, dry, ventilate and bath are modes, at most one at a time:
//     a mode's key turns it on and the other modes off, or turns it off when
//     it is already on;
//   - powerOff turns every mode off and leaves the light as it is.
// Function names are the command names in opple-fe6300.json.
class OppleFE6300State {
 public:
  static constexpr size_t FUNCTION_COUNT = 6;

  // Applies one key press by command name. Returns false for a name that is
  // neither a function nor powerOff (state unchanged).
  bool apply(const String& command);

  // False for an unknown name.
  bool isOn(const String& function) const;

  // "light:on heat:off ...", for the console.
  String describe() const;

 private:
  struct Function {
    const char* name;
    bool mode;  // One of the mutually exclusive modes; powerOff clears it.
    bool on;
  };
  Function _functions[FUNCTION_COUNT] = {
    {"heat", true, false},
    {"blow", true, false},
    {"light", false, false},
    {"bath", true, false},
    {"dry", true, false},
    {"ventilate", true, false},
  };

  void _clearModes();

  Function* _find(const String& name);
  const Function* _find(const String& name) const;
};

#endif  // OppleFE6300State_h
