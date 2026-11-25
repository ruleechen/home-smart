#ifndef VictorFile_h
#define VictorFile_h

#include <FS.h>

namespace Victor {

  class VictorFile {
   public:
    VictorFile();
    explicit VictorFile(File file);
    ~VictorFile();

    // whether the file is open
    operator bool() const;
    bool operator!() const;

    void close();

    // read/write
    size_t available();
    int read();
    size_t read(uint8_t* buf, size_t len);
    size_t write(uint8_t val);
    size_t write(const uint8_t* buf, size_t len);

    // print
    size_t print(const String& s);
    size_t print(const char* s);
    size_t println(const String& s);
    size_t println(const char* s);

    // position
    bool seek(size_t pos);
    size_t position();
    size_t size();

    // info
    String name();
    bool isDirectory();
    bool isFile();

    // expose the underlying Arduino File (Stream/Print) for ArduinoJson, etc.
    File& file();

   private:
    File _file;
  };

} // namespace Victor

#endif // VictorFile_h
