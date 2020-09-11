#ifndef TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP
#define TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP

namespace tfutils {

class TensorflowConvert : public AbstractCommand {
private:
  constexpr static Jint COMMAND_WIDTH = 224;
  constexpr static Jint COMMAND_HEIGHT = 224;

  constexpr static Jchar FORMAT_LINUX_OUTPUT[] = "%s/%s";
  constexpr static Jchar FORMAT_LINUX_CONVERT[] = "convert -resize %dx%d %s %s";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    Jint width = COMMAND_WIDTH;
    Jint height = COMMAND_HEIGHT;
    UP<File> file(new File());

    if (v->getLength() < 2)
      return -1;

    auto &&indir = (*v)[0];
    auto &&oudir = (*v)[1];

    if (v->getLength() > 2)
      width = std::stoi((*v)[2]);
    else if (v->getLength() > 3)
      height = std::stoi((*v)[3]);

    if ((!File::isExist(indir)) || (!File::isExist(oudir)))
      return -1;
    if ((File::isFile(indir)) || (File::isFile(oudir)))
      return -1;

    auto &&dir = file->getFilesInDirectory(indir);
    Program prog(dir.size());
    for (auto &&p : dir) {
      auto &&name = p->getName().data();
      auto &&path = p->getAbstractPath().data();

      auto &&output = String::format(FORMAT_LINUX_OUTPUT, (*v)[1], name);
      auto &&command = String::format(FORMAT_LINUX_CONVERT, width, height, path, output.data());

      System sys(command);
      prog.updateOne();
    }
    return 0;
  }
};

class TensorflowReplace : public AbstractCommand {
private:
  constexpr static Jint SIZE_BUFFER = 128;
  constexpr static Jint SIZE_ARGS = 8;

  constexpr static Jchar SYMBOL_MARK = '^';
  constexpr static Jchar SYMBOL_PATH = '$';
  constexpr static Jchar MODEL_READ[] = "rb";
  constexpr static Jchar MODEL_WRITE[] = "wb";
  constexpr static Jchar NAME_TEMP[] = "temp.csv";

  constexpr static Jchar TARGET_TRAIN[] = "TRAIN";
  constexpr static Jchar TARGET_VALIDATION[] = "VALIDATION";
  constexpr static Jchar TARGET_TEST[] = "TEST";

  Jchar mBuffer[SIZE_BUFFER];

  FILE *mOldFile;
  FILE *mNewFile;
  UP<ICommandArgs> mArgs;

public:
  explicit TensorflowReplace(Jchar const *v)
      : AbstractCommand(v), mBuffer(), mOldFile(), mNewFile(), mArgs(new CommandArgs<SIZE_ARGS>()) {
  }

  ~TensorflowReplace() override {
    if (this->mOldFile != nullptr)
      fclose(this->mOldFile);
    if (this->mNewFile != nullptr)
      fclose(this->mNewFile);
  }

  Jint execute(UP<ICommandArgs> const &v) override {
    Jint i = 0;
    Jint retLen = 0;
    Jfloat counts = 0;
    Jfloat current = 0;
    SP<AbstractCommand> command;

    if (v->getLength() < 2)
      return -1;

    auto &&filePath = (*v)[0];
    auto &&symbol = (*v)[1];
    auto &&symbolLen = strlen(symbol);
    this->mOldFile = fopen(filePath, MODEL_READ);
    this->mNewFile = fopen(NAME_TEMP, MODEL_WRITE);
    if ((this->mOldFile == nullptr) || (this->mNewFile == nullptr))
      return -1;

    do {
      retLen = fread(this->mBuffer, 1, sizeof(this->mBuffer), this->mOldFile);
      if (retLen > 0) {
        for (i = 0; i < retLen; ++i) {
          if (this->mBuffer[i] == SYMBOL_MARK)
            ++counts;
        }
      }
    } while (retLen == sizeof(this->mBuffer));
    fseek(this->mOldFile, 0, SEEK_SET);

    do {
      retLen = fread(this->mBuffer, 1, sizeof(this->mBuffer), this->mOldFile);
      if (retLen > 0) {
        for (i = 0; i < retLen; ++i) {
          if (this->mBuffer[i] == SYMBOL_PATH) {
            fwrite(symbol, symbolLen, 1, this->mNewFile);
          } else if (this->mBuffer[i] == SYMBOL_MARK) {
            Jint targetLen = 0;
            Jchar const *target = nullptr;
            auto &&proportion = current / counts;

            if (proportion <= 0.8)
              target = TARGET_TRAIN;
            else if ((proportion > 0.8) && (proportion <= 0.9))
              target = TARGET_VALIDATION;
            else
              target = TARGET_TEST;

            targetLen = strlen(target);
            fwrite(target, targetLen, 1, this->mNewFile);
            ++current;
          } else {
            fwrite(&this->mBuffer[i], 1, 1, this->mNewFile);
          }
        }

        fflush(this->mNewFile);
      }
    } while (retLen == sizeof(this->mBuffer));

    this->mArgs->push(filePath);
    command = make<OSRemove>();
    if (command->execute(this->mArgs) != 0)
      return -1;

    this->mArgs->clean();
    this->mArgs->push(NAME_TEMP);
    this->mArgs->push(filePath);
    command = make<OSCopy>();
    if (command->execute(this->mArgs) != 0)
      return -1;

    this->mArgs->clean();
    this->mArgs->push(NAME_TEMP);
    command = make<OSRemove>();
    return command->execute(this->mArgs);
  }
};

class TensorflowCSV : public AbstractCommand {
private:
  constexpr static Jchar SURRFIX[] = ".xml";
  constexpr static Jchar SYMBOL_MARK[] = "^";
  constexpr static Jchar SYMBOL_PATH[] = "$";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    Jchar const *target = nullptr;

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
      auto &&filename = xml->getFilename().data();

      for (auto &&object : xml->getObjects()) {
        auto &&name = object.getName().data();
        auto &&minX = object.getBndbox().getMinX();
        auto &&minY = object.getBndbox().getMinY();
        auto &&maxX = object.getBndbox().getMaxX();
        auto &&maxY = object.getBndbox().getMaxY();

        target = target == nullptr ? name : target;
        auto &&format = GoogleCloudCSVFormat(width, height, minX, minY, maxX, maxY, filename,
                                             target, SYMBOL_MARK, SYMBOL_PATH);
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

  static void converCrop(SP<LabelImageXML> const &v) {
    LabelImageConver<CROP_TOP_10> top10(v);
    LabelImageConver<CROP_TOP_20> top20(v);
    LabelImageConver<CROP_TOP_30> top30(v);
    LabelImageConver<CROP_TOP_40> top40(v);

    LabelImageConver<CROP_BOTTOM_10> bottom10(v);
    LabelImageConver<CROP_BOTTOM_20> bottom20(v);
    LabelImageConver<CROP_BOTTOM_30> bottom30(v);
    LabelImageConver<CROP_BOTTOM_40> bottom40(v);

    LabelImageConver<CROP_LEFT_10> left10(v);
    LabelImageConver<CROP_LEFT_20> left20(v);
    LabelImageConver<CROP_LEFT_30> left30(v);
    LabelImageConver<CROP_LEFT_40> left40(v);

    LabelImageConver<CROP_RIGHT_10> right10(v);
    LabelImageConver<CROP_RIGHT_20> right20(v);
    LabelImageConver<CROP_RIGHT_30> right30(v);
    LabelImageConver<CROP_RIGHT_40> right40(v);

    LabelImageConver<CROP_LEFT_10 | CROP_TOP_10> lt11(v);
    LabelImageConver<CROP_LEFT_10 | CROP_TOP_20> lt12(v);
    LabelImageConver<CROP_LEFT_20 | CROP_TOP_10> lt21(v);
    LabelImageConver<CROP_LEFT_20 | CROP_TOP_20> lt22(v);

    LabelImageConver<CROP_RIGHT_10 | CROP_TOP_10> rt11(v);
    LabelImageConver<CROP_RIGHT_10 | CROP_TOP_20> rt12(v);
    LabelImageConver<CROP_RIGHT_20 | CROP_TOP_10> rt21(v);
    LabelImageConver<CROP_RIGHT_20 | CROP_TOP_20> rt22(v);

    LabelImageConver<CROP_LEFT_10 | CROP_BOTTOM_10> lb11(v);
    LabelImageConver<CROP_LEFT_10 | CROP_BOTTOM_20> lb12(v);
    LabelImageConver<CROP_LEFT_20 | CROP_BOTTOM_10> lb21(v);
    LabelImageConver<CROP_LEFT_20 | CROP_BOTTOM_20> lb22(v);

    LabelImageConver<CROP_RIGHT_10 | CROP_BOTTOM_10> rb11(v);
    LabelImageConver<CROP_RIGHT_10 | CROP_BOTTOM_20> rb12(v);
    LabelImageConver<CROP_RIGHT_20 | CROP_BOTTOM_10> rb21(v);
    LabelImageConver<CROP_RIGHT_20 | CROP_BOTTOM_20> rb22(v);

    LabelImageConver<CROP_RIGHT_10 | CROP_LEFT_10> rl10(v);
    LabelImageConver<CROP_RIGHT_20 | CROP_LEFT_20> rl20(v);

    LabelImageConver<CROP_TOP_10 | CROP_BOTTOM_10> tb10(v);
    LabelImageConver<CROP_TOP_20 | CROP_BOTTOM_20> tb20(v);

    LabelImageConver<CROP_TOP_10 | CROP_BOTTOM_10 | CROP_RIGHT_10 | CROP_LEFT_10> tblr10(v);
    LabelImageConver<CROP_TOP_20 | CROP_BOTTOM_20 | CROP_RIGHT_20 | CROP_LEFT_20> tblr20(v);
  }

  static void convertOffset(SP<LabelImageXML> const &v) {
    LabelImageConver<OFFSET_TOP_10> top10(v);
    LabelImageConver<OFFSET_TOP_20> top20(v);

    LabelImageConver<OFFSET_BOTTOM_10> bottom10(v);
    LabelImageConver<OFFSET_BOTTOM_20> bottom20(v);

    LabelImageConver<OFFSET_LEFT_10> left10(v);
    LabelImageConver<OFFSET_LEFT_20> left20(v);

    LabelImageConver<OFFSET_RIGHT_10> right10(v);
    LabelImageConver<OFFSET_RIGHT_20> right20(v);

    LabelImageConver<OFFSET_TOP_10 | OFFSET_LEFT_10> tl10(v);
    LabelImageConver<OFFSET_TOP_20 | OFFSET_LEFT_20> tl20(v);

    LabelImageConver<OFFSET_TOP_20 | OFFSET_RIGHT_10> tr10(v);
    LabelImageConver<OFFSET_TOP_20 | OFFSET_RIGHT_20> tr20(v);

    LabelImageConver<OFFSET_BOTTOM_10 | OFFSET_LEFT_10> bl10(v);
    LabelImageConver<OFFSET_BOTTOM_20 | OFFSET_LEFT_20> bl20(v);

    LabelImageConver<OFFSET_BOTTOM_10 | OFFSET_RIGHT_10> br10(v);
    LabelImageConver<OFFSET_BOTTOM_20 | OFFSET_RIGHT_20> br20(v);
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

      LabelImageXMLExporter ex(xml, out);
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

        LabelImageXMLExporter ex(xml, newPath);
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
