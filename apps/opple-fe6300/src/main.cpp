#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <Console.h>
#include <OppleFE6300.h>
#include <OppleFE6300State.h>

using namespace Victor;

namespace {

  // ESP-12F HSPI: SCK=GPIO14, MISO=GPIO12, MOSI=GPIO13. See README for wiring.
  constexpr OppleFE6300::Pins RF_PINS = {
    4,   // CS:  NodeMCU D2
    5,   // PKT: NodeMCU D1
    16,  // RST: NodeMCU D0
  };

  OppleFE6300 radio(RF_PINS);

  // Inferred from presses on the own remote and from here; starts all off.
  // Presses of other remotes are logged but do not count: they drive other
  // heaters (A3BF5701 belongs to the second bathroom heater).
  OppleFE6300State heater;

  String hex(const uint8_t* bytes, size_t length) {
    static const char DIGITS[] = "0123456789ABCDEF";
    String text;
    text.reserve(length * 2);
    for (size_t i = 0; i < length; i++) {
      text += DIGITS[bytes[i] >> 4];
      text += DIGITS[bytes[i] & 0x0F];
    }
    return text;
  }

  void onPressed(const __FlashStringHelper* source, const String& command) {
    if (heater.apply(command)) {
      console.log()
        .bracket(F("state"))
        .section(source, heater.describe());
    }
  }

  String hex16(uint16_t value) {
    const uint8_t bytes[] = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value)};
    return hex(bytes, 2);
  }

  String hex32(uint32_t value) {
    const uint8_t bytes[] = {
      static_cast<uint8_t>(value >> 24), static_cast<uint8_t>(value >> 16),
      static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value),
    };
    return hex(bytes, 4);
  }

  // 2.4 GHz WiFi next to the LT8920 can blind it: channel 74 is 2476 MHz.
  String wifiState() {
    const auto mode = WiFi.getMode();
    if (mode == WIFI_OFF) {
      return F("off");
    }
    return String(mode == WIFI_STA ? F("sta") : mode == WIFI_AP ? F("ap") : F("ap+sta")) +
      F(" ch") + String(WiFi.channel()) +
      (WiFi.status() == WL_CONNECTED ? F(" connected") : F(""));
  }

  void logStatus() {
    const auto d = radio.diagnostics();
    console.log()
      .bracket(F("status"))
      .section(F("ready"), String(d.ready))
      .section(F("pkt"), String(d.pktPin))
      .section(F("r7"), hex16(d.channelRegister))
      .section(F("r41"), hex16(d.packetConfig))
      .section(F("r48"), hex16(d.status))
      .section(F("r52"), hex16(d.fifoControl))
      .section(F("rssi"), String(d.rssi))
      .section(F("rx"), String(d.rx.packets))
      .section(F("copies"), String(d.rx.lastCopies))
      .section(F("crc"), String(d.rx.crcErrors))
      .section(F("empty"), String(d.rx.empty))
      .section(F("oversize"), String(d.rx.oversize))
      .section(F("seq"), d.hasTxSequence ? hex16(d.txSequence) : String(F("-")))
      .section(F("state"), heater.describe())
      .section(F("rxack"), radio.rxAutoAck() ? F("on") : F("off"))
      .section(F("wifi"), wifiState());
  }

  // One line per trace run that ended at or after sinceMs, e.g.
  // "[trace] 76762+39ms rx 01D0 x19 rssi 34" or "[trace] 76810+950ms rearm x20 stuck 0 rssi 5".
  void logTrace(uint32_t sinceMs) {
    for (size_t i = 0; i < radio.traceCount(); i++) {
      const auto& e = radio.traceAt(i);
      if (static_cast<int32_t>(e.lastMs - sinceMs) < 0) {
        continue;
      }
      using Kind = OppleFE6300::TraceEvent::Kind;
      String what;
      switch (e.kind) {
        case Kind::Packets: what = String(F("rx ")) + hex16(e.sequence); break;
        case Kind::CrcError: what = F("crc"); break;
        case Kind::Empty: what = F("empty"); break;
        case Kind::Oversize: what = F("oversize"); break;
        case Kind::Rearm: what = F("rearm"); break;
      }
      what += String(F(" x")) + String(e.count);
      if (e.kind == Kind::Rearm) {
        what += String(F(" stuck ")) + String(e.stuck);
      }
      console.log()
        .bracket(F("trace"))
        .section(String(e.firstMs) + '+' + String(e.lastMs - e.firstMs) + F("ms"), what)
        .section(F("rssi"), String(e.rssiMax));
    }
  }

  constexpr uint32_t PROBE_DURATION_MS = 5000;

  void runWatch() {
    // Packets seen here are discarded: presses during watch/scan are not tracked.
    console.log().bracket(F("watch")).section(F("5s, press the remote now; presses are not tracked"));
    const auto w = radio.watch(PROBE_DURATION_MS);
    console.log()
      .bracket(F("watch"))
      .section(F("samples"), String(w.samples))
      .section(F("rssi"), String(w.rssiMin) + F("..") + String(w.rssiMax))
      .section(F("sync"), String(w.syncWords))
      .section(F("packets"), String(w.packets))
      .section(F("crc"), String(w.crcErrors))
      .section(F("pin"), String(w.pktPin));
  }

  void runScan() {
    console.log().bracket(F("scan")).section(F("5s, keep pressing the remote; presses are not tracked"));
    uint8_t maxRssi[OppleFE6300::CHANNEL_COUNT];
    radio.scan(PROBE_DURATION_MS, maxRssi);
    String line;
    for (uint8_t ch = 0; ch < OppleFE6300::CHANNEL_COUNT; ch++) {
      line += String(ch) + ':' + String(maxRssi[ch]) + ' ';
    }
    console.log().bracket(F("scan")).section(line);
  }

  // "r 48" reads register 48; "w 45 0080" writes 0x0080 to register 45.
  // Note that re-arming RX rewrites registers 7, 41 and 52.
  bool runRegister(const String& line) {
    const bool write = line.startsWith(F("w "));
    if (!write && !line.startsWith(F("r "))) {
      return false;
    }
    auto rf = radio.sdk();
    if (rf == nullptr) {
      return true;
    }
    char* end = nullptr;
    const auto reg = static_cast<uint8_t>(strtoul(line.c_str() + 2, &end, 10));
    if (write) {
      rf->writeRegister(reg, static_cast<uint16_t>(strtoul(end, nullptr, 16)));
    }
    console.log()
      .bracket(F("reg"))
      .section(String(reg), hex16(rf->readRegister(reg)));
    return true;
  }

  void runCommand(const String& name) {
    if (name == F("status")) {
      logStatus();
      return;
    }
    if (name == F("watch")) {
      runWatch();
      return;
    }
    if (name == F("trace")) {
      logTrace(0);
      return;
    }
    if (name == F("scan")) {
      runScan();
      return;
    }
    if (runRegister(name)) {
      return;
    }
    if (name == F("rxack on") || name == F("rxack off")) {
      radio.setRxAutoAck(name.endsWith(F("on")));
      console.log()
        .bracket(F("rxack"))
        .section(radio.rxAutoAck() ? F("on") : F("off"));
      return;
    }
    const auto result = radio.send(name);
    const auto d = radio.diagnostics();
    (result == OppleFE6300::TxResult::Ok ? console.log() : console.error())
      .bracket(F("tx"))
      .section(name, OppleFE6300::describe(result))
      .section(F("seq"), hex16(d.txSequence));
    if (result == OppleFE6300::TxResult::Ok) {
      onPressed(F("local"), name);
    }
  }

  // Serial monitor: type a command name from opple-fe6300.json (or "status") and
  // press Enter. The monitor sends each key as it is typed, so the line is
  // collected across loop() calls and runs on CR or LF.
  String serialLine;

  void readSerial() {
    while (Serial.available()) {
      const char c = Serial.read();
      if (c == '\r' || c == '\n') {
        serialLine.trim();
        if (serialLine.length() > 0) {
          runCommand(serialLine);
        }
        serialLine = "";
      } else if (serialLine.length() < 32) {
        serialLine += c;
      }
    }
  }

  // When the latest press was reported, for the trace of a gap.
  uint32_t previousPressMs = 0;

  void logReceived() {
    OppleFE6300::RxPacket packet;
    if (!radio.receive(packet)) {
      return;
    }
    const uint32_t sinceMs = previousPressMs;
    previousPressMs = millis();
    const auto known = packet.command != nullptr;
    console.log()
      .bracket(F("rx"))
      .section(known ? packet.command->name : String(F("unknown")))
      .section(F("remote"), hex32(packet.address()) + (packet.own ? F(" own") : F("")))
      .section(F("seq"), hex16(packet.sequence()))
      .section(F("prev copies"), String(packet.previousCopies))
      .section(F("payload"), hex(packet.payload, packet.length));
    if (packet.missed > 0) {
      // Counters show whether the radio heard garbage meanwhile (interference)
      // or nothing at all (deaf); wifi shows whether WiFi was transmitting.
      const auto& rx = radio.rxStats();
      console.error()
        .bracket(F("rx"))
        .section(F("missed presses"), String(packet.missed))
        .section(F("crc"), String(rx.crcErrors))
        .section(F("empty"), String(rx.empty))
        .section(F("oversize"), String(rx.oversize))
        .section(F("wifi"), wifiState())
        .section(packet.own ? F("state may be stale") : F("other remote"));
      // From the previous press on: did its burst stop early, was anything
      // loud on air meanwhile, did re-arms find the chip stuck in a packet.
      logTrace(sinceMs);
    }
    if (known && packet.own) {
      onPressed(F("remote"), packet.command->name);
    }
  }

} // namespace

void setup(void) {
  console.begin(115200);

  // Mounted here until the app moves to AppMain, which mounts it itself.
  // On ESP8266 a partition that fails to mount is auto-formatted, so a missing
  // filesystem image shows up below as zero commands, not as a mount failure.
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  if (radio.begin()) {
    console.log()
      .bracket(F("radio"))
      .section(F("ready"))
      .section(F("channel"), String(radio.channel()))
      .section(F("commands"), String(radio.commandCount()))
      .section(F("listening"));
    // Commands live in the filesystem image, not in the firmware.
    if (radio.commandCount() == 0) {
      console.error()
        .bracket(F("radio"))
        .section(F("no commands loaded; upload the filesystem image: pio run -e opple-fe6300 -t uploadfs"));
    }
  } else {
    console.error()
      .bracket(F("radio"))
      .section(F("register check failed; radio disabled"));
  }
  logStatus();
}

void loop(void) {
  logReceived();
  readSerial();
}
