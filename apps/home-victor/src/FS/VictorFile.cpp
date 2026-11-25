#include "VictorFile.h"

namespace Victor {

  VictorFile::VictorFile() {}

  VictorFile::VictorFile(File file) : _file(file) {}

  VictorFile::~VictorFile() {
    close();
  }

  VictorFile::operator bool() const {
    return _file;
  }

  bool VictorFile::operator!() const {
    return !_file;
  }

  void VictorFile::close() {
    if (_file) {
      _file.close();
    }
  }

  size_t VictorFile::available() {
    return _file.available();
  }

  int VictorFile::read() {
    return _file.read();
  }

  size_t VictorFile::read(uint8_t* buf, size_t len) {
    return _file.read(buf, len);
  }

  size_t VictorFile::write(uint8_t val) {
    return _file.write(val);
  }

  size_t VictorFile::write(const uint8_t* buf, size_t len) {
    return _file.write(buf, len);
  }

  size_t VictorFile::print(const String& s) {
    return _file.print(s);
  }

  size_t VictorFile::print(const char* s) {
    return _file.print(s);
  }

  size_t VictorFile::println(const String& s) {
    return _file.println(s);
  }

  size_t VictorFile::println(const char* s) {
    return _file.println(s);
  }

  bool VictorFile::seek(size_t pos) {
    return _file.seek(pos);
  }

  size_t VictorFile::position() {
    return _file.position();
  }

  size_t VictorFile::size() {
    return _file.size();
  }

  String VictorFile::name() {
    return _file.name();
  }

  bool VictorFile::isDirectory() {
    return _file.isDirectory();
  }

  bool VictorFile::isFile() {
    return _file && !_file.isDirectory();
  }

  File& VictorFile::file() {
    return _file;
  }

} // namespace Victor
