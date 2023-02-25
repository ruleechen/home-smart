#ifndef FileStorage_h
#define FileStorage_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Console.h"

  // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

  // r      Open text file for reading.  The stream is positioned at the
  //        beginning of the file.

  // r+     Open for reading and writing.  The stream is positioned at the
  //        beginning of the file.

  // w      Truncate file to zero length or create text file for writing.
  //        The stream is positioned at the beginning of the file.

  // w+     Open for reading and writing.  The file is created if it does
  //        not exist, otherwise it is truncated.  The stream is
  //        positioned at the beginning of the file.

  // a      Open for appending (writing at end of file).  The file is
  //        created if it does not exist.  The stream is positioned at the
  //        end of the file.

  // a+     Open for reading and appending (writing at end of file).  The
  //        file is created if it does not exist.  The initial file
  //        position for reading is at the beginning of the file, but
  //        output is always appended to the end of the file.

// https://arduinojson.org/v6/assistant/
// https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html
#define DEFAULT_FILE_SIZE 2048 // block size 4096

namespace Victor::Components {

  template <typename TModel>
  class FileStorage {
   public:
    FileStorage(const char* filePath);
    TModel* load();
    bool save(const TModel* model);

   protected:
    const char* _filePath;
    size_t _maxSize = DEFAULT_FILE_SIZE;
    bool _cache = true;
    TModel* _model = nullptr;
    Console _error();
    virtual void _serializeTo(const TModel* model, DynamicJsonDocument& doc);
    virtual void _deserializeFrom(TModel* model, const DynamicJsonDocument& doc);
  };

  template <typename TModel>
  FileStorage<TModel>::FileStorage(const char* filePath) {
    _filePath = filePath;
  }

  template <typename TModel>
  TModel* FileStorage<TModel>::load() {
    if (_model != nullptr) {
      if (_cache) {
        // read from cache
        return _model;
      }
      // delete current cache
      // since we are going to load new data
      delete _model;
      _model = nullptr;
    }
    // default result
    auto model = new TModel();
    // check exists
    if (LittleFS.exists(_filePath)) {
      // open file
      auto file = LittleFS.open(_filePath, "r");
      if (file) {
        // deserialize
        DynamicJsonDocument doc(_maxSize); // Store data in the heap - Dynamic Memory Allocation
        // StaticJsonDocument<DEFAULT_FILE_SIZE> doc; // Store data in the stack - Fixed Memory Allocation
        const auto error = deserializeJson(doc, file);
        if (!error) {
          // convert
          _deserializeFrom(model, doc);
        } else {
          _error().section(F("parse failed"), error.f_str());
        }
        // close
        file.close();
      } else {
        _error().section(F("open failed"));
      }
    } else {
      _error().section(F("notfound"), _filePath);
    }
    // set cache
    if (_cache) {
      _model = model;
    }
    // ret
    return model;
  }

  template <typename TModel>
  bool FileStorage<TModel>::save(const TModel* model) {
    // release cache
    if (
      _model != nullptr &&
      _model != model
    ) {
      delete _model;
      _model = nullptr;
    }
    // convert
    DynamicJsonDocument doc(_maxSize); // Store data in the heap - Dynamic Memory Allocation
    // StaticJsonDocument<DEFAULT_FILE_SIZE> doc; // Store data in the stack - Fixed Memory Allocation
    _serializeTo(model, doc);
    auto success = false;
    // open file
    auto file = LittleFS.open(_filePath, "w+");
    if (file) {
      // write
      serializeJson(doc, file);
      // close
      file.close();
      success = true;
    } else {
      _error().section(F("open(w) failed"));
    }
    return success;
  }

  template <typename TModel>
  Console FileStorage<TModel>::_error() {
    return console.error()
      .bracket(F("storage"))
      .bracket(String(_filePath));
  }

  // template <typename TModel>
  // void FileStorage<TModel>::_serializeTo(const TModel& model, DynamicJsonDocument& doc) {}

  // template <typename TModel>
  // void FileStorage<TModel>::_deserializeFrom(TModel& model, const DynamicJsonDocument& doc) {}

} // namespace Victor::Components

#endif // FileStorage_h
