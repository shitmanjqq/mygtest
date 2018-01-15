#include <cstring>
#include <string>

#include "gtest.h"
#include "gtest_message.h"
#include "gtest_string.h"
#include "gtest_port.h"

// TODO

namespace testing {

AssertionResult AssertionSuccess() {
  // std::cout << "success" << std::endl;
  return AssertionResult(true);
}

AssertionResult AssertionFailure() {
  return AssertionResult(false);
}

Test::Test() {
}

Test::~Test() {
}

void Test::Run() {
  TestBody();
}

TestResult::TestResult() {
}

TestResult::~TestResult() {
}

void TestResult::Clear() {
}

TestInfo::TestInfo(const std::string& test_case_name,
                   const std::string& name,
                   internal::TestFactoryBase* factory)
    : test_case_name_(test_case_name),
      name_(name),
      factory_(factory) {}

TestInfo::~TestInfo() {
  delete factory_;
}

void TestInfo::Run() {
  Test* test = factory_->CreateTest();
  if (test != NULL) {
    test->Run();
    delete test;
  }
}

TestCase::TestCase(const char *name)
    : name_(name) {}

TestCase::~TestCase() {
  for (auto it = test_info_list_.begin();
       it != test_info_list_.end();
       ++it) {
    delete *it;
  }
  test_info_list_.clear();
}

void TestCase::AddTestInfo(TestInfo* test_info) {
  test_info_list_.push_back(test_info);
}

void TestCase::Run() {
  for (auto it = test_info_list_.begin();
       it != test_info_list_.end();
       ++it) {
    (*it)->Run();
  }
}

UnitTest::UnitTest() {
}

UnitTest::~UnitTest() {
  for (auto it = test_cases_.begin();
       it != test_cases_.end();
       ++it) {
    delete *it;
  }
}

UnitTest* UnitTest::GetInstance() {
  static UnitTest instance;
  return &instance;
}

int UnitTest::Run() {
  for (auto it = test_cases_.begin();
       it != test_cases_.end();
       ++it) {
    (*it)->Run();
  }

  return 0;
}

TestCase* UnitTest::GetTestCase(const char *test_case_name) {
  auto it = test_cases_.begin();
  for (; it != test_cases_.end(); ++it) {
    if (0 == strcmp((*it)->name(), test_case_name)) {
      break;
    }
  }

  if (it != test_cases_.end()) {
    return *it;
  } else {
    TestCase* test_case = new TestCase(test_case_name);
    test_cases_.push_back(test_case);
    return test_case;
  }
}

void UnitTest::AddTestInfo(TestInfo* test_info) {
  GetTestCase(test_info->test_case_name())->AddTestInfo(test_info);
}


void InitGoogleTest(int* argc, char** argv) {}

namespace internal {

TestInfo* MakeAndRegisterTestInfo (
    const char* test_case_name,
    const char* name,
    TestFactoryBase* factory) {
  TestInfo* const test_info =
      new TestInfo(test_case_name, name, factory);
  GetUnitTest()->AddTestInfo(test_info);
  return test_info;
}

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
