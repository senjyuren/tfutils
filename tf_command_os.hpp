#ifndef TFUTILS_TF_COMMAND_OS_HPP
#define TFUTILS_TF_COMMAND_OS_HPP

namespace tfutils {

class OSRemove : public AbstractCommand {
public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    if (v->isEmpty())
      return -1;
    return File::remove((*v)[0]);
  }
};

class OSCopy : public AbstractCommand {
private:
  constexpr static Jchar MODEL_READ[] = "rb";
  constexpr static Jchar MODEL_WRITE[] = "wb";

  constexpr static Jint SIZE_BUFFER = 4096 * 1024;

  Jchar mBuffer[SIZE_BUFFER];

public:
  OSCopy() : AbstractCommand(), mBuffer() {}

  explicit OSCopy(Jchar const *v) : AbstractCommand(v), mBuffer() {}

  Jint execute(UP<ICommandArgs> const &v) override {
    Jint retLen = 0;

    if (v->getLength() < 2)
      return -1;

    auto &&in = fopen((*v)[0], MODEL_READ);
    auto &&ou = fopen((*v)[1], MODEL_WRITE);
    auto &&inLen = File::getSize(in);
    auto &&prog = Program(inLen);

    do {
      if ((in == nullptr) || (ou == nullptr))
        break;

      do {
        retLen = fread(this->mBuffer, 1, sizeof(this->mBuffer), in);
        if (retLen > 0) {
          fwrite(this->mBuffer, retLen, 1, ou);
          prog.update(retLen);
        }
      } while (retLen == sizeof(this->mBuffer));
    } while (false);

    if (in != nullptr)
      fclose(in);
    if (ou != nullptr)
      fclose(ou);
    return 0;
  }
};

class OSMove : public AbstractCommand {
public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    SP<AbstractCommand> cmd;
    cmd = make<OSCopy>();
    if (cmd->execute(v) != 0)
      return -1;
    cmd = make<OSRemove>();
    return cmd->execute(v);
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_OS_HPP
