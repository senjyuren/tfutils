#ifndef TFUTILS_TF_COMMAND_ADB_HPP
#define TFUTILS_TF_COMMAND_ADB_HPP

namespace tfutils {

class ADBPull : public AbstractCommand {
private:
  constexpr static Jchar COMMAND_OUT[] = ".";
  constexpr static Jchar COMMAND_FROM[] = "*.*";
  constexpr static Jchar COMMAND_GET_LIST[] = "adb shell ls %s/%s";
  constexpr static Jchar COMMAND_PULL_LOCAL[] = "adb pull %s %s";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    Jchar const *output = COMMAND_OUT;
    Jchar const *format = COMMAND_FROM;

    if (v->isEmpty())
      return -1;

    if (v->getLength() == 2)
      format = (*v)[1];
    if (v->getLength() == 3)
      output = (*v)[2];

    auto &&arg = (*v)[0];
    Log::info("ready to operate %s, format: %s, output: %s", arg, format, output);

    auto &&get = String::format(COMMAND_GET_LIST, arg, format);
    auto &&num = System(get);
    auto &&all = num.getRows();

    auto &&prg = Program(all.size());
    for (auto row : all) {
      auto &&mat = String::format(COMMAND_PULL_LOCAL, row.getRow().data(), output);
      System sys(mat);
      prg.updateOne();
    }
    return 0;
  }
};

class ADBRemove : public AbstractCommand {
private:
  constexpr static Jchar COMMAND_GET_LIST[] = "adb shell ls %s/%s";
  constexpr static Jchar COMMAND_REMOVE_LOCAL[] = "adb shell rm %s";

public:
  using AbstractCommand::AbstractCommand;

  Jint execute(UP<ICommandArgs> const &v) override {
    if (v->getLength() != 2)
      return -1;

    auto &&path = (*v)[0];
    auto &&format = (*v)[1];
    Log::info("ready to operate %s, format: %s", path, format);

    auto &&get = String::format(COMMAND_GET_LIST, path, format);
    auto &&num = System(get);
    auto &&all = num.getRows();

    auto &&prg = Program(all.size());
    for (auto row : all) {
      auto &&mat = String::format(COMMAND_REMOVE_LOCAL, row.getRow().data());
      System sys(mat);
      prg.updateOne();
    }
    return 0;
  }
};

class ADBScreenCap : public AbstractCommand {
private:
  constexpr static Jint SIZE_COMMAND_ARGS = 3;

  constexpr static Jchar COMMAND_PATH[] = "/sdcard";
  constexpr static Jchar COMMAND_FILE[] = "cap.png";
  constexpr static Jchar COMMAND_SCREENCAP[] = "adb shell screencap -p %s/%s";

  UP<ICommandArgs> mArgs;

public:
  ADBScreenCap() : AbstractCommand(), mArgs(new CommandArgs<SIZE_COMMAND_ARGS>()) {
    this->mArgs->push(COMMAND_PATH);
    this->mArgs->push(COMMAND_FILE);
  }

  explicit ADBScreenCap(Jchar const *v)
      : AbstractCommand(v), mArgs(new CommandArgs<SIZE_COMMAND_ARGS>()) {
    this->mArgs->push(COMMAND_PATH);
    this->mArgs->push(COMMAND_FILE);
  }

  Jint execute(UP<ICommandArgs> const &v) override {
    SP<AbstractCommand> executor;

    if (v->getLength() == 1)
      this->mArgs->push((*v)[0]);

    auto &&cmd = String::format(COMMAND_SCREENCAP, COMMAND_PATH, COMMAND_FILE);
    System sys(cmd);

    executor = make<ADBPull>();
    executor->execute(this->mArgs);
    executor = make<ADBRemove>();
    executor->execute(this->mArgs);
    return 0;
  }
};

} // namespace tfutils

#endif // TFUTILS_TF_COMMAND_ADB_HPP
