#ifndef TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP
#define TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP

namespace tfutils {

enum class GoogleCloudCSVTarget : Jbyte {
  TRAIN = 0x00,
  VALIDATION = 0x01,
  TEST = 0x02,
};

class GoogleCloudCSVFormat {
private:
  constexpr static Jint SIZE_ROW = 512;

  constexpr static Jchar TARGET_TRAIN[] = "TRAIN";
  constexpr static Jchar TARGET_VALIDATION[] = "VALIDATION";
  constexpr static Jchar TARGET_TEST[] = "TEST";

  constexpr static Jchar FORMAT_ROW[] = "%s,%s/%s,%s,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n";

  Jint mWidth;
  Jint mHeight;
  Jint mXmin;
  Jint mYmin;
  Jint mXmax;
  Jint mYmax;

  Jchar const *mName;
  Jchar const *mLabel;
  Jchar const *mSymbol;
  Jchar mFormatBuffer[SIZE_ROW];

public:
  GoogleCloudCSVFormat(Jint width, Jint height, Jint x1, Jint y1, Jint x2, Jint y2,
                       Jchar const *name, Jchar const *label, Jchar const *symbol)
      : mWidth(width), mHeight(height), mXmin(x1), mYmin(y1), mXmax(x2), mYmax(y2), mName(name),
        mLabel(label), mSymbol(symbol), mFormatBuffer() {}

  Jchar const *getRow(GoogleCloudCSVTarget v) {
    Jfloat x1 = 0;
    Jfloat y1 = 0;
    Jfloat x2 = 0;
    Jfloat y2 = 0;
    Jfloat x3 = 0;
    Jfloat y3 = 0;
    Jfloat x4 = 0;
    Jfloat y4 = 0;
    Jchar const *tag = nullptr;

    Jint len = 0;

    switch (v) {
    case GoogleCloudCSVTarget::TRAIN:
      tag = TARGET_TRAIN;
      break;
    case GoogleCloudCSVTarget::VALIDATION:
      tag = TARGET_VALIDATION;
      break;
    case GoogleCloudCSVTarget::TEST:
      tag = TARGET_TEST;
      break;
    }

    x1 = static_cast<Jfloat>(this->mXmin) / static_cast<Jfloat>(this->mWidth);
    y1 = static_cast<Jfloat>(this->mYmin) / static_cast<Jfloat>(this->mHeight);
    x2 = static_cast<Jfloat>(this->mXmax) / static_cast<Jfloat>(this->mWidth);
    y2 = y1;
    x3 = x2;
    y3 = static_cast<Jfloat>(this->mYmax) / static_cast<Jfloat>(this->mHeight);
    x4 = x1;
    y4 = y3;

    len = snprintf(this->mFormatBuffer, sizeof(this->mFormatBuffer), FORMAT_ROW, tag, this->mName,
                   this->mSymbol, this->mLabel, x1, y1, x2, y2, x3, y3, x4, y4);
    this->mFormatBuffer[len] = 0x00;
    return this->mFormatBuffer;
  }
};

class GoogleCloudCSV {
private:
  constexpr static Jchar FILE_OPERATION_MODEL[] = "wb";

  Jint mFileAll;
  Jint mFileCurrent;

  FILE *mCSVFile;

public:
  explicit GoogleCloudCSV(Jint all, Jchar const *outfile)
      : mFileAll(all), mFileCurrent(), mCSVFile() {
    this->mCSVFile = fopen(outfile, FILE_OPERATION_MODEL);
  }

  ~GoogleCloudCSV() {
    if (this->mCSVFile != nullptr)
      fclose(this->mCSVFile);
  }

  void add(GoogleCloudCSVFormat &v) {
    Jchar const *row = nullptr;

    auto &&prog = static_cast<Jfloat>(this->mFileCurrent) / static_cast<Jfloat>(this->mFileAll);
    if (prog <= 0.8)
      row = v.getRow(GoogleCloudCSVTarget::TRAIN);
    else if ((prog > 0.8) && (prog <= 0.9))
      row = v.getRow(GoogleCloudCSVTarget::VALIDATION);
    else if (prog > 0.9)
      row = v.getRow(GoogleCloudCSVTarget::TEST);

    auto rowLen = strlen(row);
    if (this->mCSVFile != nullptr)
      fwrite(row, rowLen, 1, this->mCSVFile);
    ++this->mFileCurrent;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP
