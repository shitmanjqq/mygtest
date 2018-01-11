#include <cstring>
#include <string>

#include "gtest.h"
#include "gtest_message.h"
#include "gtest_string.h"
#include "gtest_port.h"

namespace testing {

AssertionResult AssertionSuccess() {
  // std::cout << "success" << std::endl;
  return AssertionResult(true);
}

AssertionResult AssertionFailure() {
  return AssertionResult(false);
}

namespace internal {

bool IsTrue(bool condition) { return condition; }

std::string StringStreamToString(::std::stringstream* ss) {
  const ::std::string& str = ss->str();
  const char* const start = str.c_str();
  const char* const end = start + str.length();

  std::string result;
  result.reserve(2 * (end - start));
  for (const char* ch = start; ch != end; ++ch) {
    if (*ch == '\0') {
      result += "\\0";
    } else {
      result += *ch;
    }
  }

  return result;
}

}

Message::Message()
: ss_(new std::stringstream)
{
}

std::string Message::GetString() const {
  return internal::StringStreamToString(ss_.get());
}

AssertionResult::AssertionResult(const AssertionResult& other)
    : success_(other.success_),
      message_(other.message_.get() != NULL ?
               new ::std::string(*other.message_) :
               static_cast< ::std::string*>(NULL)) {
}

namespace internal {

namespace {

std::vector<std::string> SplitEscapedString(const std::string& str) {
  std::vector<std::string> lines;
  size_t start = 0, end = str.size();
  if (end > 2 && str[0] == '"' && str[end - 1] == '"') {
    ++start;
    --end;
  }
  bool escaped = false;
  for (size_t i = start; i + 1 < end; ++i) {
    if (escaped) {
      escaped = false;
      if (str[i] == 'n') {
        lines.push_back(str.substr(start, i - start - 1));
        start = i + 1;
      }
    } else {
      escaped = str[i] == '\\';
    }
  }
  lines.push_back(str.substr(start, end - start));
  return lines;
}

}

AssertionResult EqFailure(const char* lhs_expression,
                          const char* rhs_expression,
                          const std::string& lhs_value,
                          const std::string& rhs_value,
                          bool ignoring_case) {
  Message msg;
  msg << "Expected equality of these values:";
  msg << "\n  " << lhs_expression;
  if (lhs_value != lhs_expression) {
    msg << "\n    which is: " << lhs_value;
  }
  msg << "\n  " << rhs_expression;
  if (rhs_value != rhs_expression) {
    msg << "\n    Which is: " << rhs_value;
  }

#if 0
  if (!lhs_value.empty() && rhs_value.empty()) {
    const std::vector<std::string> lhs_lines =
        SplitEscapedString(lhs_value);
    const std::vector<std::string> rhs_lines =
        SplitEscapedString(rhs_value);
    if (lhs_lines.size() > 1 || rhs_lines.size() > 1) {
      msg << "\nWith diff:\n"
          << edit_distance::CreateUnifiedDiff(lhs_lines, rhs_lines);
    }
  }
#endif

  return AssertionFailure() << msg;
}

AssertHelper::AssertHelper(TestPartResult::Type type,
                           const char* file,
                           int line,
                           const char* message)
    : data_(new AssertHelperData(type, file, line, message)) {
}

AssertHelper::~AssertHelper() {
  delete data_;
}

/**
 * here to update the test result
 */
void AssertHelper::operator=(const Message& message) const {
  // std::cout << data_->message << std::endl;
  // std::cout << message.GetString() << std::endl;
  // UnitTest::GetInstance()->
  //   AddTestPartResult(data_->type, data_->file, data_->line,
  //                     AppendUserMessage(data_->message, message),
  //                     UnitTest::GetInstance()->impl()
  //                     ->CurrentOsStackTraceExceptTop(1)
  //                     // Skips the stack frame for this function itself.
  //                     );  // NOLINT
}

} // internal
} // testing