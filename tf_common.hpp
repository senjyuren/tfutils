#ifndef TFUTILS_TF_COMMON_HPP
#define TFUTILS_TF_COMMON_HPP

namespace tfutils {

template <class T> struct IBuffer {
  virtual ~IBuffer() = default;

  virtual void push(T v) = 0;

  virtual T operator[](Jint v) = 0;

  virtual T const *operator*() = 0;

  virtual Jint getSize() = 0;

  virtual Jint getLength() = 0;

  virtual Jbool isEmpty() = 0;

  virtual void clean() = 0;
};

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

  template <class... Args> Jchar const *input(std::string const &format, Args... args) {
    auto &&len = snprintf(this->mBuffer, sizeof(this->mBuffer), format.data(), args...);
    this->mBuffer[len] = 0x00;
    return this->mBuffer;
  }

public:
  template <class... Args> static std::string format(std::string const &format, Args... args) {
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

  template <class... Args> Jchar const *format(std::string const &format, Args... args) {
    auto &&len = snprintf(this->mBuffer, sizeof(this->mBuffer), format.data(), args...);
    this->mBuffer[len] = 0x00;
    return this->mBuffer;
  }

public:
  template <Jchar const *Tag = TAG, class... Args>
  static void info(std::string const &format, Args... args) {
    if (format.empty())
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_INFO, Tag, ret);
  }

  template <Jchar const *Tag = TAG, class... Args>
  static void debug(std::string const &format, Args... args) {
    if (format.empty())
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_DBUG, Tag, ret);
  }

  template <Jchar const *Tag = TAG, class... Args>
  static void error(std::string const &format, Args... args) {
    if (format.empty())
      return;

    auto &&ret = Log::getInstance().format(format, args...);
    printf(FORMAT_ERRO, Tag, ret);
  }
};

class Program {
private:
  constexpr static Jint BASE_NUMBER = 50;

  constexpr static Jchar CR[] = "\n";
  constexpr static Jchar SB_LEFT[] = "[";
  constexpr static Jchar SB_RIGHT[] = "]\r";
  constexpr static Jchar WL[] = "#";
  constexpr static Jchar US[] = "_";

  Jint mAll;
  Jint mCurrent;

  static void format(Jfloat all, Jfloat current) {
    Jint i = 0;
    Jint location = static_cast<Jint>(current / all * BASE_NUMBER);

    printf(SB_LEFT);
    for (i = 0; i < BASE_NUMBER; ++i) {
      if (i <= location)
        printf(WL);
      else
        printf(US);
    }

    printf(SB_RIGHT);
    if (i == BASE_NUMBER)
      printf(CR);
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

  std::string const &getRow() { return this->mRow; }
};

class System {
private:
  constexpr static Jint SIZE_CACHE = 1024;

  constexpr static Jchar ASC_LF = '\r';
  constexpr static Jchar ASC_CR = '\n';
  constexpr static Jchar MODEL_READ[] = "r";

  Jchar mCache[SIZE_CACHE];
  Jchar mCacheNew[SIZE_CACHE];
  std::list<SystemRow> mRows;

public:
  explicit System(std::string const &v) : mCache(), mCacheNew(), mRows() {
    Jint i = 0;
    Jint j = 0;
    Jint retLen = 0;

    if (v.empty())
      return;

    auto &&ctx = popen(v.data(), MODEL_READ);

    do {
      retLen = fread(this->mCache, 1, sizeof(this->mCache), ctx);
      for (i = 0; i < retLen; ++i) {
        if (this->mCache[i] == ASC_CR) {
          this->mCacheNew[j] = 0x00;
          j = 0;
          this->mRows.emplace_back(this->mCacheNew);
          continue;
        } else if (this->mCache[i] == ASC_LF) {
          continue;
        } else {
          this->mCacheNew[j] = this->mCache[i];
          ++j;
        }
      }
    } while (retLen == sizeof(this->mCache));

    pclose(ctx);
  }

  std::list<SystemRow> const &getRows() { return this->mRows; }
};

class FileAttributes {
private:
  constexpr static Jchar FORMAT_PATH[] = "%s/%s";

  std::string mFileName;
  std::string mFileBasePath;
  std::string mFileAbstractPath;

public:
  FileAttributes(std::string path, std::string name)
      : mFileName(move(name)), mFileBasePath(move(path)), mFileAbstractPath() {
    this->mFileAbstractPath =
        String::format(FORMAT_PATH, this->mFileBasePath.data(), this->mFileName.data());
  }

  std::string const &getName() { return this->mFileName; }

  std::string const &getBasePath() { return this->mFileBasePath; }

  std::string const &getAbstractPath() { return this->mFileAbstractPath; }
};

class File {
private:
  std::list<UP<FileAttributes>> mFileAttributes;

public:
  File() : mFileAttributes() {}

  static Jint getSize(FILE *v) {
    fseek(v, 0, SEEK_END);
    auto &&size = ftell(v);
    fseek(v, 0, SEEK_SET);
    return size;
  }

  static Jint remove(std::string const &v) { return ::remove(v.data()); }

  static Jbool isFile(std::string const &v) { return (!std::filesystem::is_directory(v)); }

  static Jbool isExist(std::string const &v) { return std::filesystem::exists(v); }

  template <Jchar const *Suffix = nullptr>
  std::list<UP<FileAttributes>> const &getFilesInDirectory(std::string const &v) {
    std::filesystem::directory_iterator iterator(v);

    for (auto &&entry : iterator) {
      auto &&path = entry.path().parent_path().string();
      auto &&name = entry.path().filename().string();
      auto &&exte = entry.path().extension().string();
      if ((Suffix == nullptr) || (exte == Suffix))
        this->mFileAttributes.emplace_back(new FileAttributes(path, name));
    }

    return this->mFileAttributes;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMON_HPP
