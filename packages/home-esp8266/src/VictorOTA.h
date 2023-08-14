#ifndef VictorOTA_h
#define VictorOTA_h

#include <functional>
#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include "Console.h"
#include "GlobalHelpers.h"
#include "OtaStorage/OtaStorage.h"

namespace Victor::Components {

  enum OtaType {
    OTA_ALL    = 0,
    OTA_SKETCH = 1,
    OTA_FS     = 2,
  };

  class VictorOTA {
   public:
    void setup(const char* settingFile = "/ota.json");
    String getCurrentVersion() const;
    String checkNewVersion();
    void update(const String& version, const OtaType type);
    void trigger(const OtaType type);

   private:
    OtaStorage* _storage = nullptr;
    void _updateSketch();
    void _updateFileSystem();
    void _handleStart();
    void _handleEnd();
    void _handleProgress(int progress, int total);
    void _handleError(int error);
    static Console _log();
  };

  // global
  extern VictorOTA victorOTA;

} // namespace Victor::Components

#endif // VictorOTA_h
