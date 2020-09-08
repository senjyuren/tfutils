#include "tf_core.hpp"

class Help : public tfutils::AbstractCommand {
private:
  constexpr static Jchar CONTENT[] = "os commands                                         \n"
                                     "  cp a.txt b.txt                                    \n"
                                     "  mv a.txt b.txt                                    \n"
                                     "  rm a.txt                                          \n"
                                     "adb commands                                        \n"
                                     "  pull /sdcard [*.jpeg] [.]                         \n"
                                     "  remove /sdcard *.jpeg                             \n"
                                     "  cap [.]                                           \n"
                                     "tensorflow commands                                 \n"
                                     "  convert /in_directory /out_directory [224] [224]  \n"
                                     "  replace a.csv gs://                               \n"
                                     "  csv /in_directory a.csv [target]                  \n";

public:
  using tfutils::AbstractCommand::AbstractCommand;

  Jbool isHelp() override { return true; }

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

constexpr Jchar COMMAND_TENSORFLOW_CSV[] = "csv";
constexpr Jchar COMMAND_TENSORFLOW_REPLACE[] = "replace";
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
  executor.add(new tfutils::TensorflowCSV(COMMAND_TENSORFLOW_CSV));
  executor.add(new tfutils::TensorflowConvert(COMMAND_TENSORFLOW_CONVERT));
  executor.add(new tfutils::TensorflowReplace(COMMAND_TENSORFLOW_REPLACE));
  return executor.execute();
}
