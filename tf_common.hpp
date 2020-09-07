#ifndef TFUTILS_TF_COMMON_HPP
#define TFUTILS_TF_COMMON_HPP

namespace tfutils {

class String {
private:
  constexpr static Jint SIZE_BUFFER = 1024;

  Jchar mBuffer[SIZE_BUFFER];

  String() : mBuffer() {}

  static String &getInstance() {
    static String *obj = nullptr;
    if (obj == nullptr)
      obj = new String();
    return (*obj);
  }

  template <class... Args> Jchar const *input(Jchar const *format, Args... args) {
    auto &&len = snprintf(this->mBuffer, sizeof(this->mBuffer), format, args...);
    this->mBuffer[len] = 0x00;
    return this->mBuffer;
  }

public:
  template <class... Args> static std::string format(Jchar const *format, Args... args) {
    if (format == nullptr)
      return nullptr;
    return String::getInstance().input(format, args...);
  }
};

class Log {
private:
  constexpr static Jint SIZE_BUFFER = 1024;

  constexpr static Jchar TAG[] = "senjyuren";
  constexpr static Jchar FORMAT_INFO[] = "[INFO][%s][%s]\n";
  constexpr static Jchar FORMAT_DBUG[] = "[DBUG][%s][%s]\n";
  constexpr static Jchar FORMAT_ERRO[] = "[ERRO][%s][%s]\n";

  Jchar mBuffer[SIZE_BUFFER];

  Log() : mBuffer() {}

  static Log &getInstance() {
    static Log *obj = nullptr;
    if (obj == nullptr)
      obj = new Log();
    return (*obj);
  }

  template <class... Args> Jchar const *format(Jchar const *format, Args... args) {
    auto &&len = snprintf(this->mBuffer, sizeof(this->mBuffer), format, args...);
    this->mBuffer[len] = 0x00;
    return this->mBuffer;
  }

public:
  template <Jchar const *Tag = TAG, class... Args>
  static void info(Jchar const *format, Args... args) {
    if (format == nullptr)
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_INFO, Tag, ret);
  }

  template <Jchar const *Tag = TAG, class... Args>
  static void debug(Jchar const *format, Args... args) {
    if (format == nullptr)
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_DBUG, Tag, ret);
  }

  template <Jchar const *Tag = TAG, class... Args>
  static void error(Jchar const *format, Args... args) {
    if (format == nullptr)
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_ERRO, Tag, ret);
  }
};

class Program {
private:
  constexpr static Jint BASE_NUMBER = 50;

  Jint mAll;
  Jint mCurrent;

  template <Jint Number = BASE_NUMBER> static void format(Jfloat all, Jfloat current) {
    Jint i = 0;
    Jint location = static_cast<Jint>(current / all * Number);

    printf("\r");
    printf("[");
    for (i = 0; i < Number; ++i) {
      if (i <= location)
        printf("#");
      else
        printf("_");
    }

    printf("]");
    if (i == Number)
      printf("\n");
  }

public:
  explicit Program(Jint v) : mAll(v), mCurrent() {}

  void updateOne() { Program::format(this->mAll, (++this->mCurrent)); }

  void update(Jint v) { Program::format(this->mAll, (this->mCurrent += v)); }
};

class SystemRow {
private:
  std::string mRow;

public:
  explicit SystemRow(Jchar const *v) : mRow(v) {}

  Jchar const *getRow() { return this->mRow.data(); }
};

class System {
private:
  constexpr static Jint SIZE_CACHE = 1024;

  Jchar mCache[SIZE_CACHE];
  Jchar mCacheNew[SIZE_CACHE];
  std::list<SystemRow> mRows;

public:
  explicit System(Jchar const *v) : mCache(), mCacheNew(), mRows() {
    Jint i = 0;
    Jint j = 0;
    Jint retLen = 0;

    if (v == nullptr)
      return;

    auto &&ctx = _popen(v, "r");

    do {
      retLen = fread(this->mCache, 1, sizeof(this->mCache), ctx);
      for (i = 0; i < retLen; ++i) {
        if (this->mCache[i] == '\n') {
          this->mCacheNew[j] = 0x00;
          j = 0;
          this->mRows.emplace_back(this->mCacheNew);
          continue;
        } else if (this->mCache[i] == '\r') {
          continue;
        } else {
          this->mCacheNew[j] = this->mCache[i];
          ++j;
        }
      }
    } while (retLen == sizeof(this->mCache));

    _pclose(ctx);
  }

  std::list<SystemRow> const &getRows() { return this->mRows; }
};

class File {
public:
  static Jint getSize(FILE *v) {
    fseek(v, 0, SEEK_END);
    auto &&size = ftell(v);
    fseek(v, 0, SEEK_SET);
    return size;
  }

  static Jint remove(Jchar const *v) { return ::remove(v); }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMON_HPP
