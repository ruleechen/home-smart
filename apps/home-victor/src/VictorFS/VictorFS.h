#ifndef VictorFS_h
#define VictorFS_h

#include <FS.h>

#include "VictorFS/VictorFile.h"

namespace Victor {

  class VictorFS {
   public:
    VictorFS();
    ~VictorFS();

    // Bind the concrete VictorFS instance (ESP8266/ESP32 LittleFS / SPIFFS, etc.)
    // Call once in setup(), e.g.: victorFS.begin(LittleFS);
    bool begin(FS& fs);

    bool exists(const char* path);
    VictorFile open(const char* path, const char* mode = "r");
    bool remove(const char* path);
    bool rename(const char* path, const char* newPath);

    bool isInitialized() const;

   private:
    FS* _fs = nullptr;
    bool _initialized = false;
  };

  // global
  extern VictorFS victorFS;

} // namespace Victor

#endif // VictorFS_h
