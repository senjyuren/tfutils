#ifndef TFUTILS_TF_COMMAND_HPP
#define TFUTILS_TF_COMMAND_HPP

namespace tfutils {

struct ICommandArgs {
  virtual ~ICommandArgs() = default;

  virtual void add(Jchar const *v) = 0;

  virtual Jchar *operator[](Jint v) = 0;

  virtual Jint getSize() = 0;

  virtual Jint getLength() = 0;

  virtual Jbool isEmpty() = 0;

  virtual void clean() = 0;
};

template <Jint Size> class CommandArgs : public ICommandArgs {
private:
  Jint mArgsSize;
  Jint mArgsLength;
  Jchar const *mArgs[Size];

public:
  CommandArgs() : mArgsSize(Size), mArgsLength(), mArgs() {}

  void add(Jchar const *v) override {
    if (this->mArgsLength >= this->mArgsSize)
      return;

    this->mArgs[this->mArgsLength] = v;
    ++this->mArgsLength;
  }

  Jchar *operator[](Jint v) override { return const_cast<Jchar *>(this->mArgs[v]); }

  Jint getSize() override { return this->mArgsSize; }

  Jint getLength() override { return this->mArgsLength; }

  Jbool isEmpty() override { return (this->mArgsLength == 0); }

  void clean() override { this->mArgsLength = 0; }
};

class AbstractCommand {
private:
  Jchar const *mName;

public:
  AbstractCommand() : mName() {}

  explicit AbstractCommand(Jchar const *v) : mName(v) {}

  virtual ~AbstractCommand() = default;

  virtual Jchar const *getName() { return this->mName; }

  virtual Jint execute(UP<ICommandArgs> const &v) = 0;
};

class CommandExecutor {
private:
  constexpr static Jint SIZE_COMMAND_ARGS = 1024;

  Jint mArgc;
  Jchar const *const *mArgs;

  UP<ICommandArgs> mCommandArgs;
  std::list<UP<AbstractCommand>> mCommands;

public:
  template <Jint CacheSize = SIZE_COMMAND_ARGS>
  CommandExecutor(Jint argc, Jchar const *const *args)
      : mArgc(argc), mArgs(args), mCommandArgs(new CommandArgs<CacheSize>()), mCommands() {}

  void add(AbstractCommand *v) { this->mCommands.emplace_back(v); }

  Jint execute() {
    Jint i = 0;
    Jint ret = -1;

    if (this->mArgc < 2)
      return -1;

    auto &&name = this->mArgs[1];
    this->mCommandArgs->clean();
    for (i = 2; i < this->mArgc; ++i)
      this->mCommandArgs->add(this->mArgs[i]);

    for (auto &&command : this->mCommands) {
      if (strcmp(command->getName(), name) != 0)
        continue;

      ret = command->execute(this->mCommandArgs);
      break;
    }

    return ret;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_HPP