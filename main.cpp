#include "tf_core.hpp"

class Help : public tfutils::AbstractCommand {
private:
  constexpr static Jchar CONTENT[] = "os commands                         \n"
                                     "  cp a.txt b.txt                    \n"
                                     "  mv a.txt b.txt                    \n"
                                     "  rm a.txt                          \n"
                                     "adb commands                        \n"
                                     "  pull /sdcard [*.jpeg] [.]         \n"
                                     "  remove /sdcard *.jpeg             \n"
                                     "  cap [.]                           \n"
                                     "tensorflow commands                 \n";

public:
  using tfutils::AbstractCommand::AbstractCommand;

  Jint execute(const UP<tfutils::ICommandArgs> &v) override {
    printf(CONTENT);
    return 0;
  }
};

constexpr Jchar COMMAND_HELP[] = "help";
constexpr Jchar COMMAND_COPY[] = "cp";
constexpr Jchar COMMAND_MOVE[] = "mv";
constexpr Jchar COMMAND_REMOVE[] = "rm";

constexpr Jchar COMMAND_ADB_PULL[] = "pull";
constexpr Jchar COMMAND_ADB_REMOVE[] = "rm";
constexpr Jchar COMMAND_ADB_SCREENSHOT[] = "ss";

constexpr Jchar COMMAND_TENSORFLOW_FORMAT[] = "format";
constexpr Jchar COMMAND_TENSORFLOW_CONVERT[] = "convert";

Jint main(Jint argc, Jchar *args[]) {
  tfutils::CommandExecutor executor(argc, args);
  executor.add(new Help(COMMAND_HELP));
  executor.add(new tfutils::OSCopy(COMMAND_COPY));
  executor.add(new tfutils::OSMove(COMMAND_MOVE));
  executor.add(new tfutils::OSRemove(COMMAND_REMOVE));

  // adb commands
  executor.add(new tfutils::ADBPull(COMMAND_ADB_PULL));
  executor.add(new tfutils::ADBRemove(COMMAND_ADB_REMOVE));
  executor.add(new tfutils::ADBScreenCap(COMMAND_ADB_SCREENSHOT));

  // tensorflow commands
  executor.add(new tfutils::TensorflowFormat(COMMAND_TENSORFLOW_FORMAT));
  executor.add(new tfutils::TensorflowConvert(COMMAND_TENSORFLOW_CONVERT));
  return executor.execute();
}
