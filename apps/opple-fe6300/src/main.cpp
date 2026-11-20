#include <Arduino.h>
#include <LittleFS.h>
#include <Console.h>
#include <OppleFE6300.h>
#include <OppleFE6300State.h>
#include <OppleHelper.h>

using namespace Victor;
using OppleHelper::hex;
using OppleHelper::hex16;
using OppleHelper::hex32;

namespace {

  // ESP-12F Hardware SPI (HSPI):
  //   SCK=GPIO14
  //   MISO=GPIO12
  //   MOSI=GPIO13
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

  void onPressed(const __FlashStringHelper* source, const String& command) {
    if (heater.apply(command)) {
      console.log()
        .bracket(F("state"))
        .section(source, heater.describe());
    }
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
      .section(F("state"), heater.describe());
  }

  // One line per trace run that ended at or after sinceMs, e.g.
  // "[trace] 76762+39ms: rx 01D0 x19 rssi: 34" or "[trace] 76810+950ms: rearm x20 stuck 0 rssi: 5".
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
        default: what = F("unknown"); break;
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

  // When the latest press was reported, for the trace of a gap.
  uint32_t previousPressMs = 0;

  // The serial port blocks once its 128-byte FIFO is full, and the receiver
  // is only re-armed from loop(): a trace of dozens of lines printed right
  // away would deafen it for the rest of the burst and any press right after.
  // So a gap's trace waits until nothing has been received for this long.
  constexpr uint32_t TRACE_QUIET_MS = 300;
  bool tracePending = false;
  uint32_t traceSinceMs = 0;  // Start of the earliest gap not printed yet.

  // Idle polling spins without sleeping: without this the loop would busy-wait
  // as fast as possible, burning CPU and starving the ESP8266 background tasks.
  // 1 ms is far below the 50 ms re-arm and the ~13 ms burst period, so pacing
  // does not change what the receiver does.
  constexpr uint32_t LOOP_PACE_MS = 1;

  void logPendingTrace() {
    if (tracePending && millis() - radio.lastPacketMs() >= TRACE_QUIET_MS) {
      tracePending = false;
      logTrace(traceSinceMs);
    }
  }

  void logReceived() {
    const auto before = radio.rxStats();
    OppleFE6300::RxPacket packet;
    const bool reported = radio.receive(packet);
    // A gap is trace-worthy when the press was skipped (this side may have
    // missed it, or the remote jumped the counter) or when the burst carried
    // error packets (CRC / empty / oversize): the last one does not always come
    // with a skip, so it has to count on its own.
    const bool gapActivity = packet.skipped > 0 ||
      before.crcErrors != radio.rxStats().crcErrors ||
      before.empty != radio.rxStats().empty ||
      before.oversize != radio.rxStats().oversize;
    const uint32_t sinceMs = previousPressMs;
    // From the previous press on: did its burst stop early, was anything loud
    // on air meanwhile, did re-arms find the chip stuck in a packet.
    // Printed by logPendingTrace() once the air is quiet; gaps in a row share
    // one trace from the earliest.
    if (gapActivity && !tracePending) {
      tracePending = true;
      traceSinceMs = sinceMs;
    }
    if (!reported) {
      // A burst repeat can carry error events; the gap belongs to the press
      // either way, so the armed trace above still holds.
      return;
    }
    previousPressMs = millis();
    const auto known = packet.key != nullptr;
    auto line = console.log()
      .bracket(F("rx"))
      .section(known ? packet.key->name : String(F("unknown")))
      .section(F("remote"), hex32(packet.address()))
      .section(F("seq"), hex16(packet.sequence()))
      .section(F("prev copies"), String(packet.previousCopies));
    if (!known) {
      // The raw bytes only matter for a key code not in opple-fe6300.json yet.
      line.section(F("payload"), hex(packet.payload, packet.length));
    }
    if (packet.skipped > 0) {
      // Not an error: mostly the remote skipping values on rapid presses. The
      // counters and the trace show whether this side missed presses instead.
      const auto& rx = radio.rxStats();
      console.log()
        .bracket(F("rx"))
        .section(F("counter skipped"), String(packet.skipped))
        .section(F("crc"), String(rx.crcErrors))
        .section(F("empty"), String(rx.empty))
        .section(F("oversize"), String(rx.oversize))
        .section(packet.own ? F("own remote") : F("other remote"));
    }
    if (known && packet.own) {
      onPressed(F("remote"), packet.key->name);
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
  logPendingTrace();
  delay(LOOP_PACE_MS);
}
