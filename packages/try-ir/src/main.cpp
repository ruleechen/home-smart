#include <Arduino.h>

#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

const uint16_t kRecvPin = 5;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance; // kTolerance is normally 25%

IRrecv receiver(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup(void) {
  Serial.begin(115200);
  #if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    receiver.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
  receiver.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  receiver.enableIRIn();  // Start the receiver
}

void loop(void) {
  if (receiver.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);

    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow) {
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    }

    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");

    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance) {
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    }

    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));

    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) {
      Serial.println(D_STR_MESGDESC ": " + description);
    }

    // Feed the WDT as the text output can take a while to print.
    yield();

    // Output the results as source code
    Serial.println(resultToSourceCode(&results));

    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}
