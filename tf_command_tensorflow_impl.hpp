#ifndef TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP
#define TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP

namespace tfutils {

class GoogleCloudCSVFormat {
private:
  constexpr static Jint SIZE_ROW = 512;
  constexpr static Jchar FORMAT_ROW[] = "%s,%s/%s,%s,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n";

  Jint mWidth;
  Jint mHeight;
  Jint mXmin;
  Jint mYmin;
  Jint mXmax;
  Jint mYmax;

  std::string mName;
  std::string mLabel;
  std::string mSymbolMark;
  std::string mSymbolPath;
  Jchar mFormatBuffer[SIZE_ROW];

public:
  GoogleCloudCSVFormat(Jint width, Jint height, Jint x1, Jint y1, Jint x2, Jint y2,
                       std::string name, std::string label, std::string symbolMark,
                       std::string symbolPath)
      : mWidth(width), mHeight(height), mXmin(x1), mYmin(y1), mXmax(x2), mYmax(y2),
        mName(move(name)), mLabel(move(label)), mSymbolMark(move(symbolMark)),
        mSymbolPath(move(symbolPath)), mFormatBuffer() {}

  std::string getRow() {
    Jfloat x1 = 0;
    Jfloat y1 = 0;
    Jfloat x2 = 0;
    Jfloat y2 = 0;
    Jfloat x3 = 0;
    Jfloat y3 = 0;
    Jfloat x4 = 0;
    Jfloat y4 = 0;

    x1 = static_cast<Jfloat>(this->mXmin) / static_cast<Jfloat>(this->mWidth);
    y1 = static_cast<Jfloat>(this->mYmin) / static_cast<Jfloat>(this->mHeight);
    x2 = static_cast<Jfloat>(this->mXmax) / static_cast<Jfloat>(this->mWidth);
    y2 = y1;
    x3 = x2;
    y3 = static_cast<Jfloat>(this->mYmax) / static_cast<Jfloat>(this->mHeight);
    x4 = x1;
    y4 = y3;

    auto &&len = snprintf(this->mFormatBuffer, sizeof(this->mFormatBuffer), FORMAT_ROW,
                          this->mSymbolMark.c_str(), this->mSymbolPath.c_str(), this->mName.c_str(),
                          this->mLabel.c_str(), x1, y1, x2, y2, x3, y3, x4, y4);
    this->mFormatBuffer[len] = 0x00;
    return this->mFormatBuffer;
  }
};

class GoogleCloudCSV {
private:
  constexpr static Jchar FILE_OPERATION_MODEL[] = "wb";

  FILE *mCSVFile;

public:
  explicit GoogleCloudCSV(Jint all, Jchar const *outfile) : mCSVFile() {
    this->mCSVFile = fopen(outfile, FILE_OPERATION_MODEL);
  }

  ~GoogleCloudCSV() {
    if (this->mCSVFile != nullptr)
      fclose(this->mCSVFile);
  }

  void add(GoogleCloudCSVFormat &v) {
    if (this->mCSVFile != nullptr)
      fwrite(v.getRow().data(), v.getRow().size(), 1, this->mCSVFile);
  }
};

class LabelImageXML;

class LabelImageXMLSize {
private:
  Jint mWidth;
  Jint mHeight;
  Jint mDepth;

public:
  friend LabelImageXML;

  LabelImageXMLSize() : mWidth(), mHeight(), mDepth() {}

  [[nodiscard]] Jint getWidth() const { return this->mWidth; }

  [[nodiscard]] Jint getHeight() const { return this->mHeight; }

  [[nodiscard]] Jint getDepth() const { return this->mDepth; }
};

class LabelImageXMLSource {
private:
  std::string mDatabase;

public:
  friend LabelImageXML;

  LabelImageXMLSource() : mDatabase() {}

  [[nodiscard]] std::string const &getDatabase() const { return this->mDatabase; }
};

class LabelImageXMLBndbox {
private:
  Jint mMinX;
  Jint mMinY;
  Jint mMaxX;
  Jint mMaxY;

public:
  friend LabelImageXML;

  LabelImageXMLBndbox() : mMinX(), mMinY(), mMaxX(), mMaxY() {}

  [[nodiscard]] Jint getMinX() const { return this->mMinX; }

  [[nodiscard]] Jint getMinY() const { return this->mMinY; }

  [[nodiscard]] Jint getMaxX() const { return this->mMaxX; }

  [[nodiscard]] Jint getMaxY() const { return this->mMaxY; }

  void setMinX(Jint v) { this->mMinX = v; }

  void setMinY(Jint v) { this->mMinY = v; }

  void setMaxX(Jint v) { this->mMaxX = v; }

  void setMaxY(Jint v) { this->mMaxY = v; }
};

class LabelImageXMLObject {
private:
  std::string mName;
  std::string mPose;

  Jint mTruncated;
  Jint mDifficult;

  LabelImageXMLBndbox mBndbox;

public:
  friend LabelImageXML;

  LabelImageXMLObject() : mName(), mPose(), mTruncated(), mDifficult(), mBndbox() {}

  [[nodiscard]] std::string const &getName() const { return this->mName; }

  [[nodiscard]] std::string const &getPose() const { return this->mPose; }

  [[nodiscard]] Jint getTruncated() const { return this->mTruncated; }

  [[nodiscard]] Jint getDifficult() const { return this->mDifficult; }

  [[nodiscard]] LabelImageXMLBndbox const &getBndbox() const { return this->mBndbox; }

  void setName(std::string const &v) { this->mName = v; }

  void setBndbox(LabelImageXMLBndbox const &v) { this->mBndbox = v; }
};

class LabelImageXMLTarget {
public:
  constexpr static Jchar ROOT[] = "annotation";

  constexpr static Jchar FOLDER[] = "folder";
  constexpr static Jchar FILENAME[] = "filename";
  constexpr static Jchar PATH[] = "path";
  constexpr static Jchar SEGMENTED[] = "segmented";

  constexpr static Jchar SOURCE[] = "source";
  constexpr static Jchar S_DATABASE[] = "database";

  constexpr static Jchar SIZE[] = "size";
  constexpr static Jchar S_WIDTH[] = "width";
  constexpr static Jchar S_HEIGHT[] = "height";
  constexpr static Jchar S_DEPTH[] = "depth";

  constexpr static Jchar OBJECT[] = "object";
  constexpr static Jchar O_NAME[] = "name";
  constexpr static Jchar O_POSE[] = "pose";
  constexpr static Jchar O_TRUNCATED[] = "truncated";
  constexpr static Jchar O_DIFFICULT[] = "difficult";
  constexpr static Jchar O_BNDBOX[] = "bndbox";
  constexpr static Jchar O_B_XMIN[] = "xmin";
  constexpr static Jchar O_B_YMIN[] = "ymin";
  constexpr static Jchar O_B_XMAX[] = "xmax";
  constexpr static Jchar O_B_YMAX[] = "ymax";
};

class LabelImageXML {
private:
  constexpr static Jint SIZE_BUFFER = 256;
  constexpr static Jchar MODEL_READ[] = "rb";

  FILE *mXml;
  Jchar mBuffer[SIZE_BUFFER];
  std::string mXmlContent;

  std::string mFolder;
  std::string mFilename;
  std::string mPath;

  LabelImageXMLSource mSource;
  LabelImageXMLSize mSize;

  Jint mSegmented;
  std::list<LabelImageXMLObject> mObjects;

public:
  explicit LabelImageXML(std::string const &v)
      : mXml(), mBuffer(), mXmlContent(), mFolder(), mFilename(), mPath(), mSource(), mSize(),
        mSegmented(), mObjects() {
    Jint i = 0;
    Jint retLen = 0;

    QDomDocument document;

    if (v.empty())
      return;

    this->mXml = fopen(v.c_str(), MODEL_READ);
    if (this->mXml == nullptr)
      return;

    do {
      retLen = fread(this->mBuffer, 1, sizeof(this->mBuffer), this->mXml);
      if (retLen > 0)
        this->mXmlContent.append(this->mBuffer, retLen);
    } while (retLen == sizeof(this->mBuffer));

    document.setContent(QString(this->mXmlContent.data()));
    auto &&root = document.documentElement();

    this->mFolder = root.firstChildElement(LabelImageXMLTarget::FOLDER).text().toStdString();
    this->mFilename = root.firstChildElement(LabelImageXMLTarget::FILENAME).text().toStdString();
    this->mPath = root.firstChildElement(LabelImageXMLTarget::PATH).text().toStdString();
    this->mSegmented = root.firstChildElement(LabelImageXMLTarget::SEGMENTED).text().toInt();

    auto &&source = root.firstChildElement(LabelImageXMLTarget::SOURCE);
    this->mSource.mDatabase =
        source.firstChildElement(LabelImageXMLTarget::S_DATABASE).text().toStdString();

    auto &&size = root.firstChildElement(LabelImageXMLTarget::SIZE);
    this->mSize.mWidth = size.firstChildElement(LabelImageXMLTarget::S_WIDTH).text().toInt();
    this->mSize.mHeight = size.firstChildElement(LabelImageXMLTarget::S_HEIGHT).text().toInt();
    this->mSize.mDepth = size.firstChildElement(LabelImageXMLTarget::S_DEPTH).text().toInt();

    auto &&objects = root.elementsByTagName(LabelImageXMLTarget::OBJECT);
    for (i = 0; i < objects.size(); ++i) {
      auto &&object = objects.at(i);
      auto &&bndbox = object.firstChildElement(LabelImageXMLTarget::O_BNDBOX);
      auto &&temp = LabelImageXMLObject();

      temp.mName = object.firstChildElement(LabelImageXMLTarget::O_NAME).text().toStdString();
      temp.mPose = object.firstChildElement(LabelImageXMLTarget::O_POSE).text().toStdString();
      temp.mTruncated = object.firstChildElement(LabelImageXMLTarget::O_TRUNCATED).text().toInt();
      temp.mDifficult = object.firstChildElement(LabelImageXMLTarget::O_DIFFICULT).text().toInt();

      temp.mBndbox.mMinX = bndbox.firstChildElement(LabelImageXMLTarget::O_B_XMIN).text().toInt();
      temp.mBndbox.mMinY = bndbox.firstChildElement(LabelImageXMLTarget::O_B_YMIN).text().toInt();
      temp.mBndbox.mMaxX = bndbox.firstChildElement(LabelImageXMLTarget::O_B_XMAX).text().toInt();
      temp.mBndbox.mMaxY = bndbox.firstChildElement(LabelImageXMLTarget::O_B_YMAX).text().toInt();

      this->mObjects.push_back(temp);
    }
  };

  ~LabelImageXML() {
    if (this->mXml != nullptr)
      fclose(this->mXml);
  }

  [[nodiscard]] std::string const &getFolder() const { return this->mFolder; }

  [[nodiscard]] std::string const &getFilename() const { return this->mFilename; }

  [[nodiscard]] std::string const &getPath() const { return this->mPath; }

  [[nodiscard]] LabelImageXMLSource const &getSource() const { return this->mSource; }

  [[nodiscard]] LabelImageXMLSize const &getSize() const { return this->mSize; }

  [[nodiscard]] Jint getSegmented() const { return this->mSegmented; }

  std::list<LabelImageXMLObject> &getObjects() { return this->mObjects; }

  void setFilename(std::string const &v) { this->mFilename = v; }

  void setPath(std::string const &v) { this->mPath = v; }

  void addObject(LabelImageXMLObject const &v) { this->mObjects.push_back(v); }
};

enum LabelImageCoverCrop : Juint {
  CROP_BASE = 0x01,
  CROP_TOP_10 = CROP_BASE,
  CROP_TOP_20 = CROP_BASE << 1u,
  CROP_TOP_30 = CROP_BASE << 2u,
  CROP_TOP_40 = CROP_BASE << 3u,

  CROP_BOTTOM_10 = CROP_BASE << 4u,
  CROP_BOTTOM_20 = CROP_BASE << 5u,
  CROP_BOTTOM_30 = CROP_BASE << 6u,
  CROP_BOTTOM_40 = CROP_BASE << 7u,

  CROP_LEFT_10 = CROP_BASE << 8u,
  CROP_LEFT_20 = CROP_BASE << 9u,
  CROP_LEFT_30 = CROP_BASE << 10u,
  CROP_LEFT_40 = CROP_BASE << 11u,

  CROP_RIGHT_10 = CROP_BASE << 12u,
  CROP_RIGHT_20 = CROP_BASE << 13u,
  CROP_RIGHT_30 = CROP_BASE << 14u,
  CROP_RIGHT_40 = CROP_BASE << 15u,

  OFFSET_TOP_10 = CROP_BASE << 16u,
  OFFSET_TOP_20 = CROP_BASE << 17u,

  OFFSET_BOTTOM_10 = CROP_BASE << 18u,
  OFFSET_BOTTOM_20 = CROP_BASE << 19u,

  OFFSET_LEFT_10 = CROP_BASE << 20u,
  OFFSET_LEFT_20 = CROP_BASE << 21u,

  OFFSET_RIGHT_10 = CROP_BASE << 22u,
  OFFSET_RIGHT_20 = CROP_BASE << 23u,
};

class LabelImageConver {
private:
  SP<LabelImageXML> mXml;

  template <Juint CropFun> static Jint top(Jint minY, Jint maxY) {
    auto &&ret = minY;
    auto &&stepY = (maxY - minY) / 20;
    if (CropFun & (CROP_TOP_10 | OFFSET_TOP_10))
      ret += stepY;
    else if (CropFun & (CROP_TOP_20 | OFFSET_TOP_20))
      ret += stepY * 2;
    else if (CropFun & CROP_TOP_30)
      ret += stepY * 3;
    else if (CropFun & CROP_TOP_40)
      ret += stepY * 4;
    else if (CropFun & OFFSET_BOTTOM_10)
      ret -= stepY;
    else if (CropFun & OFFSET_BOTTOM_20)
      ret -= stepY * 2;
    return ret;
  }

  template <Juint CropFun> static Jint bottom(Jint minY, Jint maxY) {
    auto &&ret = maxY;
    auto &&stepY = (maxY - minY) / 20;
    if (CropFun & (CROP_BOTTOM_10 | OFFSET_BOTTOM_10))
      ret -= stepY;
    else if (CropFun & (CROP_BOTTOM_20 | OFFSET_BOTTOM_20))
      ret -= stepY * 2;
    else if (CropFun & CROP_BOTTOM_30)
      ret -= stepY * 3;
    else if (CropFun & CROP_BOTTOM_40)
      ret -= stepY * 4;
    else if (CropFun & OFFSET_TOP_10)
      ret += stepY;
    else if (CropFun & OFFSET_TOP_20)
      ret += stepY * 2;
    return ret;
  }

  template <Juint CropFun> static Jint left(Jint minX, Jint maxX) {
    auto &&ret = minX;
    auto &&stepX = (maxX - minX) / 20;
    if (CropFun & (CROP_LEFT_10 | OFFSET_LEFT_10))
      ret += stepX;
    else if (CropFun & (CROP_LEFT_20 | OFFSET_LEFT_20))
      ret += stepX * 2;
    else if (CropFun & CROP_LEFT_30)
      ret += stepX * 3;
    else if (CropFun & CROP_LEFT_40)
      ret += stepX * 4;
    else if (CropFun & OFFSET_RIGHT_10)
      ret -= stepX;
    else if (CropFun & OFFSET_RIGHT_20)
      ret -= stepX * 2;
    return ret;
  }

  template <Juint CropFun> static Jint right(Jint minX, Jint maxX) {
    auto &&ret = maxX;
    auto &&stepX = (maxX - minX) / 20;
    if (CropFun & (CROP_RIGHT_10 | OFFSET_RIGHT_10))
      ret -= stepX;
    else if (CropFun & (CROP_RIGHT_20 | OFFSET_RIGHT_20))
      ret -= stepX * 2;
    else if (CropFun & CROP_RIGHT_30)
      ret -= stepX * 3;
    else if (CropFun & CROP_RIGHT_40)
      ret -= stepX * 4;
    else if (CropFun & OFFSET_LEFT_10)
      ret += stepX;
    else if (CropFun & OFFSET_LEFT_20)
      ret += stepX * 2;
    return ret;
  }

public:
  explicit LabelImageConver(SP<LabelImageXML> v) : mXml(move(v)) {}

  template <Juint Crop> void execute() {
    Jint minX = 0;
    Jint minY = 0;
    Jint maxX = 0;
    Jint maxY = 0;

    auto bndbox = this->mXml->getObjects().front().getBndbox();
    auto object = this->mXml->getObjects().front();

    auto &&width = this->mXml->getSize().getWidth();
    auto &&height = this->mXml->getSize().getHeight();
    auto &&minXt = this->mXml->getObjects().front().getBndbox().getMinX();
    auto &&minYt = this->mXml->getObjects().front().getBndbox().getMinY();
    auto &&maxXt = this->mXml->getObjects().front().getBndbox().getMaxX();
    auto &&maxYt = this->mXml->getObjects().front().getBndbox().getMaxY();

    auto &&bunchTop = CROP_TOP_10 | CROP_TOP_20 | CROP_TOP_30 | CROP_TOP_40 | OFFSET_TOP_10 |
                      OFFSET_TOP_20 | OFFSET_BOTTOM_10 | OFFSET_BOTTOM_20;
    auto &&bunchBottom = CROP_BOTTOM_10 | CROP_BOTTOM_20 | CROP_BOTTOM_30 | CROP_BOTTOM_40 |
                         OFFSET_TOP_10 | OFFSET_TOP_20 | OFFSET_BOTTOM_10 | OFFSET_BOTTOM_20;
    auto &&bunchLeft = CROP_LEFT_10 | CROP_LEFT_20 | CROP_LEFT_30 | CROP_LEFT_40 | OFFSET_LEFT_10 |
                       OFFSET_LEFT_20 | OFFSET_RIGHT_10 | OFFSET_RIGHT_20;
    auto &&bunchRight = CROP_RIGHT_10 | CROP_RIGHT_20 | CROP_RIGHT_30 | CROP_RIGHT_40 |
                        OFFSET_LEFT_10 | OFFSET_LEFT_20 | OFFSET_RIGHT_10 | OFFSET_RIGHT_20;

    if (Crop & bunchTop)
      minY = top<Crop>(minYt, maxYt);
    if (Crop & bunchBottom)
      maxY = bottom<Crop>(minYt, maxYt);
    if (Crop & bunchLeft)
      minX = left<Crop>(minXt, maxXt);
    if (Crop & bunchRight)
      maxX = right<Crop>(minXt, maxXt);

    if ((minY < 0) || (maxY > height) || (minX < 0) || (maxX > width))
      return;

    if (minX != 0)
      bndbox.setMinX(minX);
    if (minY != 0)
      bndbox.setMinY(minY);
    if (maxX != 0)
      bndbox.setMaxX(maxX);
    if (maxY != 0)
      bndbox.setMaxY(maxY);

    object.setBndbox(bndbox);
    this->mXml->addObject(object);
  }
};

class LabelImageXMLExporter {
private:
  constexpr static Jint SIZE_SPACE = 4;

  constexpr static Jchar MODEL_WRITE[] = "wb";

  FILE *mOutputFile;
  SP<LabelImageXML> mInputXmls;
  std::string mOutputPath;

  static void stepAnnotation(QDomDocument &doc, QDomElement &root, SP<LabelImageXML> const &in) {
    auto &&folder = doc.createElement(LabelImageXMLTarget::FOLDER);
    auto &&filename = doc.createElement(LabelImageXMLTarget::FILENAME);
    auto &&path = doc.createElement(LabelImageXMLTarget::PATH);
    auto &&segmented = doc.createElement(LabelImageXMLTarget::SEGMENTED);

    auto &&folderV = doc.createTextNode(in->getFolder().data());
    folder.appendChild(folderV);
    auto &&filenameV = doc.createTextNode(in->getFilename().data());
    filename.appendChild(filenameV);
    auto &&pathV = doc.createTextNode(in->getPath().data());
    path.appendChild(pathV);
    auto &&segmentedV = doc.createTextNode(std::to_string(in->getSegmented()).data());
    segmented.appendChild(segmentedV);

    root.appendChild(folder);
    root.appendChild(filename);
    root.appendChild(path);
    root.appendChild(segmented);
  }

  static void stepSource(QDomDocument &doc, QDomElement &root, SP<LabelImageXML> const &in) {
    auto &&source = doc.createElement(LabelImageXMLTarget::SOURCE);
    auto &&database = doc.createElement(LabelImageXMLTarget::S_DATABASE);

    auto &&databaseV = doc.createTextNode(in->getSource().getDatabase().data());
    database.appendChild(databaseV);

    source.appendChild(database);
    root.appendChild(source);
  }

  static void stepSize(QDomDocument &doc, QDomElement &root, SP<LabelImageXML> const &in) {
    auto &&size = doc.createElement(LabelImageXMLTarget::SIZE);
    auto &&width = doc.createElement(LabelImageXMLTarget::S_WIDTH);
    auto &&height = doc.createElement(LabelImageXMLTarget::S_HEIGHT);
    auto &&depth = doc.createElement(LabelImageXMLTarget::S_DEPTH);

    auto &&widthV = doc.createTextNode(std::to_string(in->getSize().getWidth()).data());
    width.appendChild(widthV);
    auto &&heightV = doc.createTextNode(std::to_string(in->getSize().getHeight()).data());
    height.appendChild(heightV);
    auto &&depthV = doc.createTextNode(std::to_string(in->getSize().getDepth()).data());
    depth.appendChild(depthV);

    size.appendChild(width);
    size.appendChild(height);
    size.appendChild(depth);
    root.appendChild(size);
  }

  static void stepObjects(QDomDocument &doc, QDomElement &root, SP<LabelImageXML> const &in) {
    auto &&objects = in->getObjects();

    for (auto &&object : objects) {
      auto &&tagObj = doc.createElement(LabelImageXMLTarget::OBJECT);

      auto &&tagName = doc.createElement(LabelImageXMLTarget::O_NAME);
      auto &&tagPose = doc.createElement(LabelImageXMLTarget::O_POSE);
      auto &&tagTruncated = doc.createElement(LabelImageXMLTarget::O_TRUNCATED);
      auto &&tagDifficult = doc.createElement(LabelImageXMLTarget::O_DIFFICULT);
      auto &&tagBndbox = doc.createElement(LabelImageXMLTarget::O_BNDBOX);

      auto &&tagXMin = doc.createElement(LabelImageXMLTarget::O_B_XMIN);
      auto &&tagYMin = doc.createElement(LabelImageXMLTarget::O_B_YMIN);
      auto &&tagXMax = doc.createElement(LabelImageXMLTarget::O_B_XMAX);
      auto &&tagYMax = doc.createElement(LabelImageXMLTarget::O_B_YMAX);

      auto &&tagNameV = doc.createTextNode(object.getName().data());
      tagName.appendChild(tagNameV);
      auto &&tagPoseV = doc.createTextNode(object.getPose().data());
      tagPose.appendChild(tagPoseV);
      auto &&tagTruncatedV = doc.createTextNode(std::to_string(object.getTruncated()).data());
      tagTruncated.appendChild(tagTruncatedV);
      auto &&tagDifficultV = doc.createTextNode(std::to_string(object.getDifficult()).data());
      tagDifficult.appendChild(tagDifficultV);

      auto &&tagXMinV = doc.createTextNode(std::to_string(object.getBndbox().getMinX()).data());
      tagXMin.appendChild(tagXMinV);
      auto &&tagYMinV = doc.createTextNode(std::to_string(object.getBndbox().getMinY()).data());
      tagYMin.appendChild(tagYMinV);
      auto &&tagXMaxV = doc.createTextNode(std::to_string(object.getBndbox().getMaxX()).data());
      tagXMax.appendChild(tagXMaxV);
      auto &&tagYMaxV = doc.createTextNode(std::to_string(object.getBndbox().getMaxY()).data());
      tagYMax.appendChild(tagYMaxV);

      tagBndbox.appendChild(tagXMin);
      tagBndbox.appendChild(tagYMin);
      tagBndbox.appendChild(tagXMax);
      tagBndbox.appendChild(tagYMax);

      tagObj.appendChild(tagName);
      tagObj.appendChild(tagPose);
      tagObj.appendChild(tagTruncated);
      tagObj.appendChild(tagDifficult);
      tagObj.appendChild(tagBndbox);
      root.appendChild(tagObj);
    }
  }

public:
  explicit LabelImageXMLExporter(SP<LabelImageXML> input, std::string output)
      : mOutputFile(), mInputXmls(move(input)), mOutputPath(move(output)) {}

  ~LabelImageXMLExporter() {
    if (this->mOutputFile != nullptr)
      fclose(this->mOutputFile);
  }

  void exported() {
    QDomDocument document;

    if (this->mOutputPath.empty())
      return;

    this->mOutputFile = fopen(this->mOutputPath.c_str(), MODEL_WRITE);
    if (this->mOutputFile == nullptr)
      return;

    auto &&root = document.createElement(LabelImageXMLTarget::ROOT);
    stepAnnotation(document, root, this->mInputXmls);
    stepSource(document, root, this->mInputXmls);
    stepSize(document, root, this->mInputXmls);
    stepObjects(document, root, this->mInputXmls);
    document.appendChild(root);

    auto &&content = document.toString(SIZE_SPACE).toStdString();
    fwrite(content.data(), content.size(), 1, this->mOutputFile);
    fflush(this->mOutputFile);
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_TENSORFLOW_IMPL_HPP
