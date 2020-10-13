#include "tf_core.hpp"

class Help : public tfutils::AbstractCommand {
private:
  constexpr static Jchar CONTENT[] =
      "os commands                                                         \n"
      "  cp a.txt b.txt                                                    \n"
      "  mv a.txt b.txt                                                    \n"
      "  rm a.txt                                                          \n"
      "adb commands                                                        \n"
      "  pull /sdcard [*.jpeg] [.]                                         \n"
      "  remove /sdcard *.jpeg                                             \n"
      "  cap [.]                                                           \n"
      "tensorflow commands                                                 \n"
      "  convert InDirectory OutDirectory [224] [224]                      \n"
      "  replace a.csv gs://                                               \n"
      "  replaceobject a.csv name value                                    \n"
      "  csv InDirectory a.csv [target]                                    \n"
      "  transform [InDirectory|a.xml] [OutDirectory|b.xml] [crop|offset]  \n"
      "  clone 0.xml 100 [1]                                               \n"
      "  check [InDirectory|a.xml] [filename|name|path|size]               \n"
      "  rotate [InDirectory|a.xml] name value                             \n";

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
constexpr Jchar COMMAND_ADB_REMOVE[] = "remove";
constexpr Jchar COMMAND_ADB_SCREENSHOT[] = "cap";

constexpr Jchar COMMAND_TENSORFLOW_CSV[] = "csv";
constexpr Jchar COMMAND_TENSORFLOW_CHECK[] = "check";
constexpr Jchar COMMAND_TENSORFLOW_CLONE[] = "clone";
constexpr Jchar COMMAND_TENSORFLOW_ROTATE[] = "rotate";
constexpr Jchar COMMAND_TENSORFLOW_REPLACE[] = "replace";
constexpr Jchar COMMAND_TENSORFLOW_CONVERT[] = "convert";
constexpr Jchar COMMAND_TENSORFLOW_TRANSFORM[] = "transform";
constexpr Jchar COMMAND_TENSORFLOW_REPLACE_OBJECT[] = "replaceobject";

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
  executor.add(new tfutils::TensorflowCheck(COMMAND_TENSORFLOW_CHECK));
  executor.add(new tfutils::TensorflowClone(COMMAND_TENSORFLOW_CLONE));
  executor.add(new tfutils::TensorflowRotate(COMMAND_TENSORFLOW_ROTATE));
  executor.add(new tfutils::TensorflowConvert(COMMAND_TENSORFLOW_CONVERT));
  executor.add(new tfutils::TensorflowReplace(COMMAND_TENSORFLOW_REPLACE));
  executor.add(new tfutils::TensorflowTransform(COMMAND_TENSORFLOW_TRANSFORM));
  executor.add(new tfutils::TensorflowReplaceObject(COMMAND_TENSORFLOW_REPLACE_OBJECT));
  return executor.execute();
}
