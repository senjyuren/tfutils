#ifndef TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP
#define TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP

namespace tfutils {

class TensorflowCheck : public AbstractCommand {
private:
  constexpr static Jchar FORMAT_FILENAME[] = "file: %s-%s-%s";
  constexpr static Jchar FORMAT_PATH[] = "file: %s-%s-%s";
  constexpr static Jchar FORMAT_SIZE[] = "file: %s-%s-%dx%d";
  constexpr static Jchar FORMAT_NAME[] = "file: %s-%s-%s";

  static void print(const SP<LabelImageXML> &xml, const std::string &mark,
                    const std::string &name) {
    if (mark == LabelImageXMLTarget::FILENAME) {
      Log::info(FORMAT_FILENAME, name.c_str(), mark.c_str(), xml->getFilename().c_str());
    } else if (mark == LabelImageXMLTarget::PATH) {
      Log::info(FORMAT_PATH, name.c_str(), mark.c_str(), xml->getPath().c_str());
    } else if (mark == LabelImageXMLTarget::SIZE) {
      Log::info(FORMAT_SIZE, name.c_str(), mark.c_str(), xml->getSize().getWidth(),
                xml->getSize().getHeight());
    } else if (mark == LabelImageXMLTarget::O_NAME) {
      for (auto &&obj : xml->getObjects())
        Log::info(FORMAT_NAME, name.c_str(), mark.c_str(), obj.getName().c_str());
    }
  }

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(const UP<ICommandArgs> &v) override {
    if (v->getLength() < 2)
      return -1;

    auto &&path = std::filesystem::path((*v)[0]);
    auto &&mark = std::string((*v)[1]);

    if (!std::filesystem::is_directory(path)) {
      auto &&xml = make<LabelImageXML>(path);
      print(xml, mark, path.filename());
    } else {
      auto &&file = make<File>();
      auto &&list = file->getFilesInDirectory(path);

      for (auto &&one : list) {
        auto &&xml = make<LabelImageXML>(one->getAbstractPath());
        print(xml, mark, one->getName());
      }
    }
    return 0;
  }
};

class TensorflowRotate : public AbstractCommand {
private:
  constexpr static Jchar FORMAT_NAME[] = "file: %s-%s-%s, replace value: %s";

  static void replace(const SP<LabelImageXML> &xml, const std::string &mark,
                      const std::string &name, const std::string &value) {
    for (auto &&obj : xml->getObjects()) {
      Log::info(FORMAT_NAME, name.c_str(), mark.c_str(), obj.getName().c_str(), value.c_str());
      obj.setName(value);
    }
  }

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(const UP<ICommandArgs> &v) override {
    if (v->getLength() < 3)
      return -1;

    auto &&path = std::filesystem::path((*v)[0]);
    auto &&mark = std::string((*v)[1]);
    auto &&value = std::string((*v)[2]);

    if (!std::filesystem::is_directory(path)) {
      auto &&xml = make<LabelImageXML>(path);
      replace(xml, mark, path.filename(), value);
    } else {
      auto &&file = make<File>();
      auto &&list = file->getFilesInDirectory(path);

      for (auto &&one : list) {
        auto &&xml = make<LabelImageXML>(one->getAbstractPath());
        replace(xml, mark, one->getName(), value);
        LabelImageXMLExporter(xml, one->getAbstractPath()).exported();
      }
    }
    return 0;
  }
};

class TensorflowClone : public AbstractCommand {
private:
  constexpr static Jchar FORMAT_FILE_NAME[] = "%d%s";
  constexpr static Jchar FORMAT_FILE_PATH[] = "%s/%s";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(const UP<ICommandArgs> &v) override {
    Juint i = 0;

    if (v->getLength() < 2)
      return -1;

    auto &&file = (*v)[0];
    auto &&length = std::strtol((*v)[1], nullptr, 10);

    if (v->getLength() == 3)
      i = std::strtol((*v)[2], nullptr, 10);

    auto &&basexml = make<LabelImageXML>(file);
    auto &&oldpre = std::filesystem::path(file).extension();
    auto &&prefix = std::filesystem::path(basexml->getFilename()).extension();
    auto &&basepath = std::filesystem::path(basexml->getPath()).parent_path();

    auto &&prog = Program(length - i);
    for (; i <= length; ++i) {
      auto &&xmlNew = *basexml;
      auto &&filename = String::format(FORMAT_FILE_NAME, i, prefix.c_str());
      xmlNew.setFilename(filename);
      xmlNew.setPath(String::format(FORMAT_FILE_PATH, basepath.c_str(), filename.c_str()));

      auto &&newfilename = String::format(FORMAT_FILE_NAME, i, oldpre.c_str());
      LabelImageXMLExporter(SP<LabelImageXML>(&xmlNew, [](LabelImageXML *) {}), newfilename)
          .exported();

      prog.updateOne();
    }

    return 0;
  }
};

class TensorflowConvert : public AbstractCommand {
private:
  constexpr static Jint COMMAND_WIDTH = 224;
  constexpr static Jint COMMAND_HEIGHT = 224;

  constexpr static Jchar FORMAT_LINUX_OUTPUT[] = "%s/%s";
  constexpr static Jchar FORMAT_LINUX_CONVERT[] = "convert -resize %dx%d %s %s";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(const UP<ICommandArgs> &v) override {
    Jint width = COMMAND_WIDTH;
    Jint height = COMMAND_HEIGHT;
    UP<File> file(new File());

    if (v->getLength() < 2)
      return -1;

    auto &&indir = (*v)[0];
    auto &&oudir = (*v)[1];

    if (v->getLength() > 3) {
      width = std::stoi((*v)[2]);
      height = std::stoi((*v)[3]);
    }

    if ((!File::isExist(indir)) || (!File::isExist(oudir)))
      return -1;
    if ((File::isFile(indir)) || (File::isFile(oudir)))
      return -1;

    auto &&dir = file->getFilesInDirectory(indir);
    Program prog(dir.size());
    for (auto &&p : dir) {
      auto &&name = p->getName();
      auto &&path = p->getAbstractPath();

      auto &&output = String::format(FORMAT_LINUX_OUTPUT, oudir, name.c_str());
      auto &&command =
          String::format(FORMAT_LINUX_CONVERT, width, height, path.c_str(), output.c_str());

      System sys(command);
      prog.updateOne();
    }
    return 0;
  }
};

class TensorflowReplace : public AbstractCommand {
private:
  FILE *mOldFile;
  FILE *mNewFile;

public:
  explicit TensorflowReplace(const Jchar *v) : AbstractCommand(v), mOldFile(), mNewFile() {}

  ~TensorflowReplace() override {
    if (this->mOldFile != nullptr)
      fclose(this->mOldFile);
    if (this->mNewFile != nullptr)
      fclose(this->mNewFile);
  }

  Jint execute(const UP<ICommandArgs> &v) override {
    std::map<std::string, Jfloat> classificationCount;
    std::map<std::string, Jfloat> classificationTotal;
    std::map<std::string, std::list<TFCSVRow *>> classification;

    if (v->getLength() < 2)
      return -1;

    auto &&file = (*v)[0];
    auto &&symbol = (*v)[1];

    auto &&inCSV = make<TFCSV>(file);
    inCSV->parse();

    auto &&prog = Program(inCSV->getRows().size() * 3);
    for (auto &&row : inCSV->getRows()) {
      row.replacePath(symbol);
      classification[row.getTarget()].emplace_back(&row);
      prog.updateOne();
    }

    for (auto &&kv : classification) {
      for (auto &&row : classification[kv.first]) {
        ++classificationTotal[row->getTarget()];
        prog.updateOne();
      }
    }

    for (auto &&kv : classification) {
      for (auto &&row : classification[kv.first]) {
        auto &&remainder =
            classificationCount[row->getTarget()] / classificationTotal[row->getTarget()];
        if (remainder < 0.8)
          row->setModel(TFCSVRow::TARGET_TRAIN);
        else if (remainder >= 0.8 && remainder < 0.9)
          row->setModel(TFCSVRow::TARGET_VALIDATION);
        else
          row->setModel(TFCSVRow::TARGET_TEST);
        ++classificationCount[row->getTarget()];
        prog.updateOne();
      }
    }

    return TFCSV::exported(inCSV, file);
  }
};

class TensorflowCSV : public AbstractCommand {
private:
  constexpr static Jchar SURRFIX[] = ".xml";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(const UP<ICommandArgs> &v) override {
    std::string target;

    UP<File> file(new File());
    SP<GoogleCloudCSV> csv = nullptr;
    SP<LabelImageXML> xml = nullptr;

    if (v->getLength() < 2)
      return -1;
    if (v->getLength() == 3)
      target = (*v)[2];

    auto &&list = file->getFilesInDirectory<SURRFIX>((*v)[0]);
    auto &&prog = Program(list.size());
    csv = make<GoogleCloudCSV>((list.size() * 3), (*v)[1]);

    for (auto &&path : list) {
      xml = make<LabelImageXML>(path->getAbstractPath());
      auto &&width = xml->getSize().getWidth();
      auto &&height = xml->getSize().getHeight();
      auto &&filename = xml->getFilename();

      for (auto &&object : xml->getObjects()) {
        auto &&name = object.getName();
        auto &&minX = object.getBndbox().getMinX();
        auto &&minY = object.getBndbox().getMinY();
        auto &&maxX = object.getBndbox().getMaxX();
        auto &&maxY = object.getBndbox().getMaxY();

        auto &&newTarget = target.empty() ? name : target;
        auto &&format =
            GoogleCloudCSVFormat(width, height, minX, minY, maxX, maxY, filename, newTarget,
                                 TFCSVRow::SYMBOL_MARK, TFCSVRow::SYMBOL_PATH);
        csv->add(format);
      }

      prog.updateOne();
    }

    return 0;
  }
};

class TensorflowTransform : public AbstractCommand {
private:
  constexpr static Jchar SURRFIX[] = ".xml";
  constexpr static Jchar FORMAT_NEW_XML_PATH[] = "%s/%s";

  constexpr static Jchar MODEL_CROP[] = "crop";
  constexpr static Jchar MODEL_OFFSET[] = "offset";

  static void converCrop(const SP<LabelImageXML> &v) {
    LabelImageConver object(v);

    object.execute<CROP_TOP_10>();
    object.execute<CROP_TOP_20>();
    object.execute<CROP_TOP_30>();
    object.execute<CROP_TOP_40>();

    object.execute<CROP_BOTTOM_10>();
    object.execute<CROP_BOTTOM_20>();
    object.execute<CROP_BOTTOM_30>();
    object.execute<CROP_BOTTOM_40>();

    object.execute<CROP_LEFT_10>();
    object.execute<CROP_LEFT_20>();
    object.execute<CROP_LEFT_30>();
    object.execute<CROP_LEFT_40>();

    object.execute<CROP_RIGHT_10>();
    object.execute<CROP_RIGHT_20>();
    object.execute<CROP_RIGHT_30>();
    object.execute<CROP_RIGHT_40>();

    object.execute<CROP_LEFT_10 | CROP_TOP_10>();
    object.execute<CROP_LEFT_10 | CROP_TOP_20>();
    object.execute<CROP_LEFT_20 | CROP_TOP_10>();
    object.execute<CROP_LEFT_20 | CROP_TOP_20>();

    object.execute<CROP_RIGHT_10 | CROP_TOP_10>();
    object.execute<CROP_RIGHT_10 | CROP_TOP_20>();
    object.execute<CROP_RIGHT_20 | CROP_TOP_10>();
    object.execute<CROP_RIGHT_20 | CROP_TOP_20>();

    object.execute<CROP_LEFT_10 | CROP_BOTTOM_10>();
    object.execute<CROP_LEFT_10 | CROP_BOTTOM_20>();
    object.execute<CROP_LEFT_20 | CROP_BOTTOM_10>();
    object.execute<CROP_LEFT_20 | CROP_BOTTOM_20>();

    object.execute<CROP_RIGHT_10 | CROP_BOTTOM_10>();
    object.execute<CROP_RIGHT_10 | CROP_BOTTOM_20>();
    object.execute<CROP_RIGHT_20 | CROP_BOTTOM_10>();
    object.execute<CROP_RIGHT_20 | CROP_BOTTOM_20>();

    object.execute<CROP_RIGHT_10 | CROP_LEFT_10>();
    object.execute<CROP_RIGHT_20 | CROP_LEFT_20>();

    object.execute<CROP_TOP_10 | CROP_BOTTOM_10>();
    object.execute<CROP_TOP_20 | CROP_BOTTOM_20>();

    object.execute<CROP_TOP_10 | CROP_BOTTOM_10 | CROP_RIGHT_10 | CROP_LEFT_10>();
    object.execute<CROP_TOP_20 | CROP_BOTTOM_20 | CROP_RIGHT_20 | CROP_LEFT_20>();
  }

  static void convertOffset(SP<LabelImageXML> const &v) {
    LabelImageConver object(v);

    object.execute<OFFSET_TOP_10>();
    object.execute<OFFSET_TOP_20>();

    object.execute<OFFSET_BOTTOM_10>();
    object.execute<OFFSET_BOTTOM_20>();

    object.execute<OFFSET_LEFT_10>();
    object.execute<OFFSET_LEFT_20>();

    object.execute<OFFSET_RIGHT_10>();
    object.execute<OFFSET_RIGHT_20>();

    object.execute<OFFSET_TOP_10 | OFFSET_LEFT_10>();
    object.execute<OFFSET_TOP_20 | OFFSET_LEFT_20>();

    object.execute<OFFSET_TOP_20 | OFFSET_RIGHT_10>();
    object.execute<OFFSET_TOP_20 | OFFSET_RIGHT_20>();

    object.execute<OFFSET_BOTTOM_10 | OFFSET_LEFT_10>();
    object.execute<OFFSET_BOTTOM_20 | OFFSET_LEFT_20>();

    object.execute<OFFSET_BOTTOM_10 | OFFSET_RIGHT_10>();
    object.execute<OFFSET_BOTTOM_20 | OFFSET_RIGHT_20>();
  }

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    SP<LabelImageXML> xml = nullptr;

    if (v->getLength() < 3)
      return -1;

    auto &&in = (*v)[0];
    auto &&out = (*v)[1];
    auto &&model = (*v)[2];

    if (File::isFile(in) && File::isFile(out)) {
      xml = make<LabelImageXML>(in);

      if (strcmp(model, MODEL_CROP) == 0)
        converCrop(xml);
      else if (strcmp(model, MODEL_OFFSET) == 0)
        convertOffset(xml);

      LabelImageXMLExporter(xml, out).exported();
      return 0;
    } else if ((!File::isFile(in)) && (!File::isFile(out))) {
      UP<File> file(new File());

      auto &&list = file->getFilesInDirectory<SURRFIX>(in);
      Program prog(list.size());
      for (auto &&f : list) {
        auto &&newPath = String::format(FORMAT_NEW_XML_PATH, out, f->getName().data());
        xml = make<LabelImageXML>(f->getAbstractPath());

        if (strcmp(model, MODEL_CROP) == 0)
          converCrop(xml);
        else if (strcmp(model, MODEL_OFFSET) == 0)
          convertOffset(xml);

        LabelImageXMLExporter(xml, newPath).exported();
        prog.updateOne();
      }
      return 0;
    } else {
      return -1;
    }
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP
