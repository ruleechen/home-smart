#include <Arduino.h>
#include <LittleFS.h>
#include <Console.h>
#include <OppleFE6300.h>
#include <OppleFE6300State.h>
#include <OppleHelper.h>
#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <arduino_homekit_server.h>

using namespace Victor;
using namespace Victor::Components;
using OppleHelper::hex;
using OppleHelper::hex16;
using OppleHelper::hex32;

AppMain* appMain = nullptr;

extern "C" homekit_characteristic_t currentHeaterState;
extern "C" homekit_characteristic_t targetHeaterState;
extern "C" homekit_characteristic_t accessoryManufacturer;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_characteristic_t accessoryModel;
extern "C" homekit_characteristic_t accessoryVersion;
extern "C" homekit_characteristic_t accessoryIdentify;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

bool isIdentified = false;

void identify(const homekit_value_t value) {
  isIdentified = !isIdentified;
  builtinLed.toggle();
}

namespace {

  // ESP-12F / NodeMCU Hardware SPI (HSPI):
  //   SCK = GPIO14 / NodeMCU D5
  //   MISO = GPIO12 / NodeMCU D6
  //   MOSI = GPIO13 / NodeMCU D7
  constexpr OppleFE6300::Pins RF_PINS = {
    4,   // CS = GPIO4 /  NodeMCU D2
    5,   // PKT = GPIO5 / NodeMCU D1
    16,  // RST = GPIO16 / NodeMCU D0
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

  // Maps a HomeKit heater target state to a heater command name. The heater
  // keys are toggles, so a target maps to a press like the web buttons do.
  String heaterCommandOf(uint8_t state) {
    switch (state) {
      case 0: return String(F("powerOff"));
      case 1: return String(F("heat"));
      case 2: return String(F("blow"));
      case 3: return String(F("dry"));
      default: return String(F("powerOff"));
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
      .section(F("sequence"), d.hasTxSequence ? hex16(d.txSequence) : String(F("-")))
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

  void logPendingTrace() {
    if (tracePending && millis() - radio.lastPacketMs() >= TRACE_QUIET_MS) {
      tracePending = false;
      logTrace(traceSinceMs);
    }
  }

  void logReceived() {
    OppleFE6300::RxPacket packet;
    if (!radio.receive(packet)) {
      return;
    }
    const uint32_t sinceMs = previousPressMs;
    previousPressMs = millis();
    const auto known = packet.key != nullptr;
    auto line = console.log()
      .bracket(F("rx"))
      .section(known ? packet.key->name : String(F("unknown")))
      .section(F("remote"), packet.remoteFormat ? hex32(packet.address()) : String(F("-")))
      .section(F("sequence"), packet.remoteFormat ? hex16(packet.sequence()) : String(F("-")))
      .section(F("prev copies"), String(packet.previousCopies));
    if (!known) {
      // The raw bytes only matter for a key code not in opple-fe6300.json yet.
      line.section(F("payload"), hex(packet.payload, packet.length));
    }
    if (packet.skipped > 0) {
      // Not an error: mostly the remote skipping values on rapid presses. The
      // sequence and the trace show whether this side missed presses instead.
      const auto& rx = radio.rxStats();
      console.log()
        .bracket(F("rx"))
        .section(F("sequence skipped"), String(packet.skipped))
        .section(F("crc"), String(rx.crcErrors))
        .section(F("empty"), String(rx.empty))
        .section(F("oversize"), String(rx.oversize))
        .section(packet.own ? F("own remote") : F("other remote"));
      // From the previous press on: did its burst stop early, was anything
      // loud on air meanwhile, did re-arms find the chip stuck in a packet.
      // Printed by logPendingTrace() once the air is quiet; gaps in a row
      // share one trace from the earliest.
      if (!tracePending) {
        tracePending = true;
        traceSinceMs = sinceMs;
      }
    }
    if (known && packet.own) {
      onPressed(F("remote"), packet.key->name);
    }
  }

} // namespace

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

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

  // Web: the buttons are key presses on the heater, so they go through the same
  // send path as the remote's; the heater ignores command names it does not know.
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    states.push_back({ .text = F("Blow"),      .value = GlobalHelpers::toOnOffName(heater.isOn(F("blow"))) });
    states.push_back({ .text = F("Heat"),      .value = GlobalHelpers::toOnOffName(heater.isOn(F("heat"))) });
    states.push_back({ .text = F("Light"),     .value = GlobalHelpers::toOnOffName(heater.isOn(F("light"))) });
    states.push_back({ .text = F("Bath"),      .value = GlobalHelpers::toOnOffName(heater.isOn(F("bath"))) });
    states.push_back({ .text = F("Dry"),       .value = GlobalHelpers::toOnOffName(heater.isOn(F("dry"))) });
    states.push_back({ .text = F("Ventilate"), .value = GlobalHelpers::toOnOffName(heater.isOn(F("ventilate"))) });
    states.push_back({ .text = F("Sequence"), .value = hex16(radio.sequence()) });
    buttons.push_back({ .text = F("Power Off"), .value = F("powerOff") });
    buttons.push_back({ .text = F("Blow"), .value = F("blow") });
    buttons.push_back({ .text = F("Heat"), .value = F("heat") });
    buttons.push_back({ .text = F("Light"), .value = F("light") });
    buttons.push_back({ .text = F("Bath"), .value = F("bath") });
    buttons.push_back({ .text = F("Dry"), .value = F("dry") });
    buttons.push_back({ .text = F("Ventilate"), .value = F("ventilate") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    console.log()
      .bracket(F("service"))
      .section(F("post"), value);
    if (heater.apply(value)) {
      const auto result = radio.send(value);
      const auto d = radio.diagnostics();
      const auto hasSentSequence = result != OppleFE6300::TxResult::NotReady &&
        result != OppleFE6300::TxResult::UnknownCommand && d.hasTxSequence;
      console.log()
        .bracket(F("send"))
        .section(F("result"), OppleFE6300::describe(result))
        .section(F("sequence"), hasSentSequence ? hex16(d.txSequence) : String(F("-")));
    }
  };

  // HomeKit: the heater target becomes a key press, sent through the same path
  // as the web buttons and the physical remote; the current state is inferred.
  targetHeaterState.setter = [](const homekit_value_t value) {
    const auto command = heaterCommandOf(static_cast<uint8_t>(value.uint8_value));
    console.log()
      .bracket(F("homekit"))
      .section(F("target"), String(static_cast<uint8_t>(value.uint8_value)));
    if (heater.apply(command)) {
      const auto result = radio.send(command);
      console.log()
        .bracket(F("send"))
        .section(F("result"), OppleFE6300::describe(result));
    }
  };
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify(identify);
}

void loop(void) {
  arduino_homekit_loop();
  logReceived();
  logPendingTrace();
  appMain->loop(-1);
}
