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

struct IString : public IBuffer<Jchar> {
  ~IString() override = default;

  virtual void append(Jchar const *v) = 0;

  virtual void append(Jchar const *v, Jint vLen) = 0;
};

template <Jint Size = 1024> class String : public IString {
private:
  Jint mSize;
  Jint mLength;
  Jchar mBuffer[Size];

  static String &getInstance() {
    static String *obj = nullptr;
    if (obj == nullptr)
      obj = new String<>();
    return (*obj);
  }

  template <class... Args> Jchar const *input(Jchar const *format, Args... args) {
    auto &&len = snprintf(this->mBuffer, sizeof(this->mBuffer), format, args...);
    this->mBuffer[len] = 0x00;
    return this->mBuffer;
  }

public:
  String() : mSize(Size), mLength(), mBuffer() {}

  explicit String(Jchar const *v) : mSize(Size), mLength(), mBuffer() {
    if (v == nullptr)
      return;

    auto &&vLen = strlen(v);
    auto &&bufferLen = (vLen + 1) > Size ? (Size - 1) : vLen;
    memcpy(this->mBuffer, v, bufferLen);
    this->mLength += bufferLen;
  }

  template <class... Args> static std::string format(Jchar const *format, Args... args) {
    return String::getInstance().input(format, args...);
  }

  Jchar operator[](Jint v) override { return this->mBuffer[v]; }

  Jchar const *operator*() override { return this->mBuffer; }

  void push(Jchar v) override {
    if (this->mLength > (Size - 1))
      return;

    this->mBuffer[this->mLength] = v;
    ++this->mLength;
    this->mBuffer[this->mLength] = 0x00;
  }

  void append(Jchar const *v) override { this->append(v, strlen(v)); }

  void append(Jchar const *v, Jint vLen) override {
    if ((v == nullptr) || (vLen <= 0))
      return;
    if (this->mLength > (Size - 1))
      return;

    auto &&bufferLen = (this->mLength + vLen + 1) > Size ? (Size - this->mLength - 1) : vLen;
    memcpy(&this->mBuffer[this->mLength], v, bufferLen);
    this->mLength += bufferLen;
    this->mBuffer[this->mLength] = 0x00;
  }

  Jint getSize() override { return this->mSize; }

  Jint getLength() override { return this->mLength; }

  Jbool isEmpty() override { return (this->mLength == 0); }

  void clean() override { this->mLength = 0; }
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

  Jchar const *getRow() { return this->mRow.data(); }
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
  explicit System(Jchar const *v) : mCache(), mCacheNew(), mRows() {
    Jint i = 0;
    Jint j = 0;
    Jint retLen = 0;

    if (v == nullptr)
      return;

    auto &&ctx = popen(v, MODEL_READ);

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
  constexpr static Jint SIZE_CACHE = 256;

  constexpr static Jchar FORMAT_PATH[] = "%s/%s";

  UP<IString> mFileName;
  UP<IString> mFileBasePath;
  SP<IString> mFileAbstractPath;

public:
  FileAttributes(Jchar const *path, Jchar const *name)
      : mFileName(new String<SIZE_CACHE>(name)), mFileBasePath(new String<SIZE_CACHE>(path)),
        mFileAbstractPath() {
    auto &&format = String<>::format(FORMAT_PATH, (**this->mFileBasePath), (**this->mFileName));
    this->mFileAbstractPath = make<String<SIZE_CACHE>>(format.data());
  }

  Jchar const *getName() { return (**this->mFileName); }

  Jchar const *getBasePath() { return (**this->mFileBasePath); }

  Jchar const *getAbstractPath() { return (**this->mFileAbstractPath); }
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

  static Jint remove(Jchar const *v) { return ::remove(v); }

  static Jbool isFile(Jchar const *v) { return (!std::filesystem::is_directory(v)); }

  static Jbool isExist(Jchar const *v) { return std::filesystem::exists(v); }

  template <Jchar const *Suffix = nullptr>
  std::list<UP<FileAttributes>> const &getFilesInDirectory(Jchar const *v) {
    std::filesystem::directory_iterator iterator(v);

    for (auto &&entry : iterator) {
      auto &&path = entry.path().parent_path().string();
      auto &&name = entry.path().filename().string();
      auto &&exte = entry.path().extension().string();
      if ((Suffix == nullptr) || (strcmp(exte.data(), Suffix) == 0))
        this->mFileAttributes.emplace_back(new FileAttributes(path.data(), name.data()));
    }

    return this->mFileAttributes;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMON_HPP
