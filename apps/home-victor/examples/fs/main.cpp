#include <Arduino.h>

#include <LittleFS.h>

#include <Console/Console.h>
#include <VictorFS/VictorFS.h>

using namespace Victor;

void setup(void) {
  console.begin(115200);

  // mount filesystem (ESP8266 / ESP32 both support LittleFS)
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
    return;
  }

  // bind to the generic filesystem abstraction layer
  victorFS.begin(LittleFS);

  const char* testPath = "/hello.txt";

  // write file
  {
    FileStream file = victorFS.open(testPath, "w");
    if (file) {
      file.println(F("Hello, home-victor!"));
      file.println(F("FileSystem works on Arduino framework."));
      file.close();
      console.log()
        .bracket(F("fs"))
        .section(F("write"), testPath);
    } else {
      console.error()
        .bracket(F("fs"))
        .section(F("open(w) failed"), testPath);
    }
  }

  // check if file exists
  if (victorFS.exists(testPath)) {
    console.log()
      .bracket(F("fs"))
      .section(F("exists"), testPath);
  }

  // read file
  {
    FileStream file = victorFS.open(testPath, "r");
    if (file) {
      console.log()
        .bracket(F("fs"))
        .section(F("read"), testPath);
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
      Serial.println();
    } else {
      console.error()
        .bracket(F("fs"))
        .section(F("open(r) failed"), testPath);
    }
  }

  // delete file (clean up after example)
  if (victorFS.remove(testPath)) {
    console.log()
      .bracket(F("fs"))
      .section(F("removed"), testPath);
  }

  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  // example runs only once in setup
  delay(1000);
}
