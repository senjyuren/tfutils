#ifndef TFUTILS_TF_COMMAND_HPP
#define TFUTILS_TF_COMMAND_HPP

namespace tfutils {

using ICommandArgs = IBuffer<const Jchar *>;

template <Jint Size> class CommandArgs : public ICommandArgs {
private:
  Jint mArgsSize;
  Jint mArgsLength;
  const Jchar *mArgs[Size];

public:
  CommandArgs() : mArgsSize(Size), mArgsLength(), mArgs() {}

  void push(const Jchar *v) override {
    if (this->mArgsLength >= this->mArgsSize)
      return;

    this->mArgs[this->mArgsLength] = v;
    ++this->mArgsLength;
  }

  const Jchar *operator[](Jint v) override { return const_cast<Jchar *>(this->mArgs[v]); }

  const Jchar *const *operator*() override { return this->mArgs; };

  Jint getLength() override { return this->mArgsLength; }

  Jbool isEmpty() override { return (this->mArgsLength == 0); }

  void clean() override { this->mArgsLength = 0; }
};

class AbstractCommand {
private:
  const Jchar *mName;

public:
  AbstractCommand() : mName() {}

  explicit AbstractCommand(const Jchar *v) : mName(v) {}

  virtual ~AbstractCommand() = default;

  virtual Jbool isHelp() { return false; }

  virtual const Jchar *getName() { return this->mName; }

  virtual Jint execute(const UP<ICommandArgs> &v) = 0;
};

class CommandExecutor {
private:
  constexpr static Jint SIZE_COMMAND_ARGS = 1024;

  Jint mArgc;
  const Jchar *const *mArgs;

  UP<ICommandArgs> mCommandArgs;
  std::list<UP<AbstractCommand>> mCommands;

public:
  template <Jint CacheSize = SIZE_COMMAND_ARGS>
  CommandExecutor(Jint argc, const Jchar *const *args)
      : mArgc(argc), mArgs(args), mCommandArgs(new CommandArgs<CacheSize>()), mCommands() {}

  void add(AbstractCommand *v) { this->mCommands.emplace_back(v); }

  Jint execute() {
    Jint i = 0;
    Jint ret = -1;

    if (this->mArgc < 2) {
      for (auto &&command : this->mCommands) {
        if (command->isHelp())
          command->execute(this->mCommandArgs);
      }
      return -1;
    }

    auto &&name = this->mArgs[1];
    this->mCommandArgs->clean();
    for (i = 2; i < this->mArgc; ++i)
      this->mCommandArgs->push(this->mArgs[i]);

    for (auto &&command : this->mCommands) {
      if (strcmp(command->getName(), name) != 0)
        continue;

      ret = command->execute(this->mCommandArgs);
      break;
    }

    if (ret != 0) {
      for (auto &&command : this->mCommands) {
        if (!command->isHelp())
          continue;
        command->execute(this->mCommandArgs);
        break;
      }
    }

    return ret;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_HPP
