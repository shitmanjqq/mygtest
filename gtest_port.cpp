#include <stdlib.h>
#include <utility>

#include "gtest_port.h"
#include "gtest_message.h"

namespace testing {
namespace internal {

const char kUnknownFile[] = "unknown file";

GTEST_API_ ::std::string FormatFileLocation(const char* file, int line) {
  const std::string file_name(file == NULL ? kUnknownFile : file);

  if (line < 0) {
    return file_name + ":";
  }

  return file_name + ":" + StreamableToString(line) + ":";
}

GTestLog::GTestLog(GTestLogSeverity severity, const char* file, int line)
    : severity_(severity) {
  const char* const marker =
      severity == GTEST_INFO ?    "[  INFO ]" :
      severity == GTEST_WARNING ? "[WARNING]" :
      severity == GTEST_ERROR ?   "[ ERROR ]" : "[ FATAL ]";
  GetStream() << ::std::endl << marker << " "
              << FormatFileLocation(file, line).c_str() << ": ";
}

GTestLog::~GTestLog() {
  GetStream() << ::std::endl;
  if (severity_ == GTEST_FATAL) {
    fflush(stderr);
    posix::Abort();
  }
}

bool BoolFromGTestEnv(const char* flag, bool default_value) {
  // TODO
  return false;
}

Int32 Int32FromGTestEnv(const char* flag, Int32 default_value) {
  // TODO
  return 1;
}

std::string StringFromGTestEnv(const char* flag, const char* default_value) {
  // TODO
  return default_value;
}

} // namespace internal
} // namespace testing