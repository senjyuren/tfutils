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
      auto &&name = p->getName();
      auto &&path = p->getAbstractPath();

      auto &&output = String<>::format(FORMAT_LINUX_OUTPUT, (*v)[1], name);
      auto &&command = String<>::format(FORMAT_LINUX_CONVERT, width, height, path, output.data());

      auto &&sys = System(command.data());
      prog.updateOne();
    }
    return 0;
  }
};

class TensorflowReplace : public AbstractCommand {
private:
  constexpr static Jint SIZE_BUFFER = 128;
  constexpr static Jint SIZE_ARGS = 8;

  constexpr static Jchar SYMBOL = '$';
  constexpr static Jchar MODEL_READ[] = "rb";
  constexpr static Jchar MODEL_WRITE[] = "wb";
  constexpr static Jchar NAME_TEMP[] = "temp.csv";

  Jchar mBuffer[SIZE_BUFFER];

  FILE *mOldFile;
  FILE *mNewFile;
  UP<ICommandArgs> mArgs;

public:
  TensorflowReplace()
      : AbstractCommand(), mBuffer(), mOldFile(), mNewFile(), mArgs(new CommandArgs<SIZE_ARGS>()) {}

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
          if (this->mBuffer[i] != SYMBOL)
            fwrite(&this->mBuffer[i], 1, 1, this->mNewFile);
          else
            fwrite(symbol, symbolLen, 1, this->mNewFile);
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
  constexpr static Jint SIZE_BUFFER = 128;
  constexpr static Jint SIZE_CONTENT = 4096;

  constexpr static Jchar MODEL[] = "rb";
  constexpr static Jchar SYMBOL[] = "$";
  constexpr static Jchar SURRFIX[] = ".xml";

  constexpr static Jchar TARGET_FILE_NAME[] = "filename";
  constexpr static Jchar TARGET_SIZE[] = "size";
  constexpr static Jchar TARGET_OBJECT[] = "object";
  constexpr static Jchar TARGET_WIDTH[] = "width";
  constexpr static Jchar TARGET_HEIGHT[] = "height";
  constexpr static Jchar TARGET_BNDBOX[] = "bndbox";
  constexpr static Jchar TARGET_NAME[] = "name";
  constexpr static Jchar TARGET_X_MIN[] = "xmin";
  constexpr static Jchar TARGET_Y_MIN[] = "ymin";
  constexpr static Jchar TARGET_X_MAX[] = "xmax";
  constexpr static Jchar TARGET_Y_MAX[] = "ymax";

  Jchar mBuffer[SIZE_BUFFER];
  UP<IString> mContent;

  QDomDocument mDocument;

public:
  explicit TensorflowCSV(Jchar const *v)
      : AbstractCommand(v), mBuffer(), mContent(new String<SIZE_CONTENT>()), mDocument() {}

  Jint execute(UP<ICommandArgs> const &v) override {
    Jint i = 0;
    Jint retLen = 0;
    Jchar const *target = nullptr;

    UP<File> file(new File());
    SP<GoogleCloudCSV> csv = nullptr;

    if (v->getLength() < 2)
      return -1;
    if (v->getLength() == 3)
      target = (*v)[2];

    auto &&list = file->getFilesInDirectory<SURRFIX>((*v)[0]);
    auto &&prog = Program(list.size());
    csv = make<GoogleCloudCSV>((list.size() * 3), (*v)[1]);
    for (auto &&path : list) {
      this->mContent->clean();
      auto &&xml = fopen(path->getAbstractPath(), MODEL);
      if (xml == nullptr)
        continue;

      do {
        retLen = fread(this->mBuffer, 1, sizeof(this->mBuffer), xml);
        if (retLen > 0)
          this->mContent->append(this->mBuffer, retLen);
      } while (retLen == sizeof(this->mBuffer));

      fclose(xml);

      this->mDocument.clear();
      this->mDocument.setContent(QString(**this->mContent));

      auto &&pointRoot = this->mDocument.documentElement();
      auto &&size = pointRoot.firstChildElement(TARGET_SIZE);
      auto &&objects = pointRoot.elementsByTagName(TARGET_OBJECT);

      auto &&filename = pointRoot.firstChildElement(TARGET_FILE_NAME).text().toStdString();
      auto &&width = std::stoi(size.firstChildElement(TARGET_WIDTH).text().toStdString());
      auto &&height = std::stoi(size.firstChildElement(TARGET_HEIGHT).text().toStdString());

      for (i = 0; i < objects.size(); ++i) {
        auto &&object = objects.at(i);
        auto &&bndbox = object.firstChildElement(TARGET_BNDBOX);

        auto &&name = object.firstChildElement(TARGET_NAME).text().toStdString();
        auto &&minX = std::stoi(bndbox.firstChildElement(TARGET_X_MIN).text().toStdString());
        auto &&minY = std::stoi(bndbox.firstChildElement(TARGET_Y_MIN).text().toStdString());
        auto &&maxX = std::stoi(bndbox.firstChildElement(TARGET_X_MAX).text().toStdString());
        auto &&maxY = std::stoi(bndbox.firstChildElement(TARGET_Y_MAX).text().toStdString());

        target = target == nullptr ? name.data() : target;
        auto &&format = GoogleCloudCSVFormat(width, height, minX, minY, maxX, maxY, filename.data(),
                                             target, SYMBOL);
        csv->add(format);
      }

      prog.updateOne();
    }

    return 0;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_TENSORFLOW_IN_HPP
