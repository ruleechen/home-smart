#include "Interrupter.h"

namespace Victor::Components {

  static void IRAM_ATTR _interruptHandler0() { _emitInterrupt(0); }
  static void IRAM_ATTR _interruptHandler1() { _emitInterrupt(1); }
  static void IRAM_ATTR _interruptHandler2() { _emitInterrupt(2); }
  static void IRAM_ATTR _interruptHandler3() { _emitInterrupt(3); }
  static void IRAM_ATTR _interruptHandler4() { _emitInterrupt(4); }
  static void IRAM_ATTR _interruptHandler5() { _emitInterrupt(5); }
  static void IRAM_ATTR _interruptHandler6() { _emitInterrupt(6); }
  static void IRAM_ATTR _interruptHandler7() { _emitInterrupt(7); }
  static void IRAM_ATTR _interruptHandler8() { _emitInterrupt(8); }
  static void IRAM_ATTR _interruptHandler9() { _emitInterrupt(9); }

  std::map<uint8_t, Interrupter*> _indexInterruptMap = {};
  std::vector<InterrupterConfig> _interrupterHandlers = {
    { .index = 0, .handler = _interruptHandler0, },
    { .index = 1, .handler = _interruptHandler1, },
    { .index = 2, .handler = _interruptHandler2, },
    { .index = 3, .handler = _interruptHandler3, },
    { .index = 4, .handler = _interruptHandler4, },
    { .index = 5, .handler = _interruptHandler5, },
    { .index = 6, .handler = _interruptHandler6, },
    { .index = 7, .handler = _interruptHandler7, },
    { .index = 8, .handler = _interruptHandler8, },
    { .index = 9, .handler = _interruptHandler9, },
  };

  void _emitInterrupt(uint8_t index) {
    if (_indexInterruptMap.count(index) > 0) {
      const auto interrupter = _indexInterruptMap[index];
      if (interrupter != nullptr) {
        interrupter->_emit();
      }
    }
  }

  Interrupter::Interrupter() {
    for (auto config : _interrupterHandlers) {
      if (_indexInterruptMap.count(config.index) == 0) {
        _indexInterruptMap[config.index] = this;
        _config = &config;
        break;
      }
    }
  }

  Interrupter::~Interrupter() {
    if (_config != nullptr) {
      if (_indexInterruptMap.count(_config->index) > 0) {
        _indexInterruptMap.erase(_config->index);
      }
      _config = nullptr;
    }
  }

  StaticInterruptHandler Interrupter::getHandler() {
    return (_config != nullptr) ? _config->handler : nullptr;
  }

  void Interrupter::_emit() {
    if (onInterrupt != nullptr) {
      onInterrupt(_config->index);
    }
  }

} // namespace Victor::Components
