#include <cstdio>
#include <cstring>
#include <string>
#include <stdarg.h>

#include "gtest.h"
#include "gtest_message.h"
#include "gtest_string.h"
#include "gtest_port.h"

// TODO

namespace testing {

using internal::ForEach;
using internal::GetElementOr;
using internal::CountIf;

namespace internal {

const char kStackTraceMarker[] = "\nStack trace:\n";

GTEST_DEFINE_int32_(
    repeat,
    internal::Int32FromGTestEnv("repeat", 1),
    "How many times to repeat each test.  Specify a negative number "
    "for repeating forever.  Useful for shaking out flaky tests.");

} // namespace internal

AssertionResult AssertionSuccess() {
  // std::cout << "success" << std::endl;
  return AssertionResult(true);
}

AssertionResult AssertionFailure() {
  return AssertionResult(false);
}

static int SumOverTestCaseList(const std::vector<TestCase*>& case_list,
                               int (TestCase::*method)() const) {
  int sum = 0;
  for (size_t i = 0; i < case_list.size(); ++i) {
    sum += (case_list[i]->*method)();
  }
  return sum;
}

static bool TestCasePassed(const TestCase* test_case) {
  return test_case->Passed();
}

static bool TestCaseFailed(const TestCase* test_case) {
  return test_case->Failed();
}

static bool ShouldRunTestCase(const TestCase* test_case) {
  return test_case->should_run();
}

namespace internal {

class TestEventRepeater : public TestEventListener {
 public:
  TestEventRepeater() : forwarding_enabled_(true) {}
  virtual ~TestEventRepeater();
  void Append(TestEventListener* listener);
  TestEventListener* Release(TestEventListener* listener);

  bool forwarding_enabled() const { return forwarding_enabled_; }
  void set_forwarding_enabled(bool enable) { forwarding_enabled_ = enable; }

  virtual void OnTestProgramStart(const UnitTest& unit_test);
  virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test);
  virtual void OnTestCaseStart(const TestCase& test_case);
  virtual void OnTestStart(const TestInfo& test_info);
  virtual void OnTestPartResult(const TestPartResult& result);
  virtual void OnTestEnd(const TestInfo& test_info);
  virtual void OnTestCaseEnd(const TestCase& test_case);
  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test);
  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);
  virtual void OnTestProgramEnd(const UnitTest& unit_test);

 private:
  bool forwarding_enabled_;
  std::vector<TestEventListener*> listeners_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestEventRepeater);
};

TestEventRepeater::~TestEventRepeater() {
  ForEach(listeners_, Delete<TestEventListener>);
}

void TestEventRepeater::Append(TestEventListener* listener) {
  listeners_.push_back(listener);
}

TestEventListener* TestEventRepeater::Release(TestEventListener* listener) {
  for (size_t i = 0; i < listeners_.size(); ++i) {
    if (listeners_[i] == listener) {
      listeners_.erase(listeners_.begin() + i);
      return listener;
    }
  }

  return NULL;
}

#define GTEST_REPEATER_METHOD_(Name, Type) \
void TestEventRepeater::Name(const Type& parameter) { \
  if (forwarding_enabled_) { \
    for (size_t i = 0; i < listeners_.size(); ++i) { \
      listeners_[i]->Name(parameter); \
    } \
  } \
}

#define GTEST_REVERSE_REPEATER_METHOD_(Name, Type) \
void TestEventRepeater::Name(const Type& parameter) { \
  if (forwarding_enabled_) { \
    for (int i = static_cast<int>(listeners_.size()) - 1; i >= 0; --i) {\
      listeners_[i]->Name(parameter); \
    } \
  } \
}

GTEST_REPEATER_METHOD_(OnTestProgramStart, UnitTest)
GTEST_REPEATER_METHOD_(OnEnvironmentsSetUpStart, UnitTest)
GTEST_REPEATER_METHOD_(OnTestCaseStart, TestCase)
GTEST_REPEATER_METHOD_(OnTestStart, TestInfo)
GTEST_REPEATER_METHOD_(OnTestPartResult, TestPartResult)
GTEST_REPEATER_METHOD_(OnEnvironmentsTearDownStart, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsTearDownEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnTestEnd, TestInfo)
GTEST_REVERSE_REPEATER_METHOD_(OnTestCaseEnd, TestCase)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsSetUpEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnTestProgramEnd, UnitTest)

#undef GTEST_REPEATER_METHOD_
#undef GTEST_REVERSE_REPEATER_METHOD_

void TestEventRepeater::OnTestIterationStart(const UnitTest& unit_test,
                                             int iteration) {
  if (forwarding_enabled_) {
    for (size_t i = 0; i < listeners_.size(); ++i) {
      listeners_[i]->OnTestIterationStart(unit_test, iteration);
    }
  }
}

void TestEventRepeater::OnTestIterationEnd(const UnitTest& unit_test,
                                           int iteration) {
  if (forwarding_enabled_) {
    for (int i = static_cast<int>(listeners_.size()) - 1; i > 0; --i) {
      listeners_[i]->OnTestIterationEnd(unit_test, iteration);
    }
  }
}

} // namespace internal

TestEventListeners::TestEventListeners()
    : repeater_(new internal::TestEventRepeater()),
      default_result_printer_(NULL),
      default_xml_generator_(NULL) {
}

TestEventListeners::~TestEventListeners() {
  delete repeater_;
}

void TestEventListeners::Append(TestEventListener* listener) {
  repeater_->Append(listener);
}

TestEventListener* TestEventListeners::Release(TestEventListener* listener) {
  if (listener == default_result_printer_)
    default_result_printer_ = NULL;
  else if (listener == default_xml_generator_)
    default_xml_generator_ = NULL;
  return repeater_->Release(listener);
}

TestEventListener* TestEventListeners::repeater() {
  return repeater_;
}

void TestEventListeners::SetDefaultResultPrinter(TestEventListener* listener) {
  if (default_result_printer_ != listener) {
    delete Release(default_result_printer_);
    default_result_printer_ = listener;
    if (listener != NULL)
      Append(listener);
  }
}

void TestEventListeners::SetDefaultXmlGenerator(TestEventListener* listener) {
  if (default_xml_generator_ != listener) {
    delete Release(default_xml_generator_);
    default_xml_generator_ = listener;
    if (listener != NULL) {
      Append(listener);
    }
  }
}

bool TestEventListeners::EventForwardingEnabled() const {
  return repeater_->forwarding_enabled();
}

void TestEventListeners::SuppressEventForwarding() {
  repeater_->set_forwarding_enabled(false);
}

static std::string FormatCountableNoun(int count,
                                       const char* singular_form,
                                       const char* plural_form) {
  return internal::StreamableToString(count) + " " +
      (count == 1 ? singular_form : plural_form);
}

static std::string FormatTestCount(int test_count) {
  return FormatCountableNoun(test_count, "test", "tests");
}

static std::string FormatTestCaseCount(int test_case_count) {
  return FormatCountableNoun(test_case_count, "test case", "test cases");
}

static const char* TestPartResultTypeToString(TestPartResult::Type type) {
  switch(type) {
    case TestPartResult::kSuccess:
      return "Success";
    case TestPartResult::kNonFatalFailure:
    case TestPartResult::kFatalFailure:
      return "Failure\n";
    default:
      return "Unknown result type";
  }
}

namespace internal {

static std::string PrintTestPartResultToString(
    const TestPartResult& test_part_result) {
  return (Message()
          << internal::FormatFileLocation(test_part_result.file_name(),
                                          test_part_result.line_number())
          << " " << TestPartResultTypeToString(test_part_result.type())
          << test_part_result.message()).GetString();
}

static void PrintTestPartResult(const TestPartResult& test_part_result) {
  const std::string& result =
      PrintTestPartResultToString(test_part_result);
  printf("%s\n", result.c_str());
  fflush(stdout);
}

enum GTestColor {
  COLOR_DEFAULT,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW
};

static const char* GetAnsiColorCode(GTestColor color) {
  switch(color) {
    case COLOR_RED:      return "1";
    case COLOR_GREEN :   return "2";
    case COLOR_YELLOW :  return "3";
    default:             return "NULL";
  }
}

bool ShouldUseColor(bool stdout_is_tty) {
  // const char* const gtest_color = GTEST_FLAG(color).c_str();
  return true;
}

static void ColoredPrintf(GTestColor color, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

#if GTEST_OS_WINDOWS_MOBILE || GTEST_OS_SYMBIAN || GTEST_OS_ZOS || \
    GTEST_OS_IOS || GTEST_OS_WINDOWS_PHONE || GTEST_OS_WINDOWS_RT
  const bool use_color = AlwaysFalse();
#else
  static const bool in_color_mode =
      ShouldUseColor(posix::IsATTY(posix::FileNo(stdout)) != 0);
  const bool use_color = in_color_mode && (color != COLOR_DEFAULT);
#endif  // GTEST_OS_WINDOWS_MOBILE || GTEST_OS_SYMBIAN || GTEST_OS_ZOS
  // The '!= 0' comparison is necessary to satisfy MSVC 7.1.

  if (!use_color) {
    vprintf(fmt, args);
    va_end(args);
    return;
  }

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE && \
    !GTEST_OS_WINDOWS_PHONE && !GTEST_OS_WINDOWS_RT && !GTEST_OS_WINDOWS_MINGW
  const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  // Gets the current text color.
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
  const WORD old_color_attrs = buffer_info.wAttributes;
  const WORD new_color = GetNewColor(color, old_color_attrs);

  // We need to flush the stream buffers into the console before each
  // SetConsoleTextAttribute call lest it affect the text that is already
  // printed but has not yet reached the console.
  fflush(stdout);
  SetConsoleTextAttribute(stdout_handle, new_color);

  vprintf(fmt, args);

  fflush(stdout);
  // Restores the text color.
  SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
  printf("\033[0;3%sm", GetAnsiColorCode(color));
  vprintf(fmt, args);
  printf("\033[m");  // Resets the terminal to default.
#endif  // GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE
  va_end(args);
}

static void PrintFullTestCommentIfPresent(const TestInfo& test_info) {
  // TODO
}

class PrettyUnitTestResultPrinter : public TestEventListener {
 public:
  PrettyUnitTestResultPrinter() {}
  static void PrintTestName(const char* test_case, const char* test) {
    printf("%s.%s", test_case, test);
  }

  virtual void OnTestProgramStart(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestCaseStart(const TestCase& test_case);
  virtual void OnTestStart(const TestInfo& test_info);
  virtual void OnTestPartResult(const TestPartResult& result);
  virtual void OnTestEnd(const TestInfo& test_info);
  virtual void OnTestCaseEnd(const TestCase& test_case);
  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);
  virtual void OnTestProgramEnd(const UnitTest& /*unit_test*/) {}

 private:
  static void PrintFailedTests(const UnitTest& unit_test);
};

void PrettyUnitTestResultPrinter::OnTestIterationStart(
    const UnitTest& unit_test, int iteration) {
  if (GTEST_FLAG(repeat) != 1)
    printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);

  // const char* const file
  ColoredPrintf(COLOR_GREEN, "[==========] ");
  printf("Running %s from %s.\n",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnEnvironmentsSetUpStart(const UnitTest&) {
  ColoredPrintf(COLOR_GREEN, "[----------] ");
  printf("Global test environment set-up.\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestCaseStart(const TestCase& test_case) {
  const std::string counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
      ColoredPrintf(COLOR_GREEN, "[----------] ");
      printf("%s from %s\n", counts.c_str(), test_case.name());
      // if ()

      fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestStart(const TestInfo& test_info) {
  ColoredPrintf(COLOR_GREEN, "[ RUN      ] ");
  PrintTestName(test_info.test_case_name(), test_info.name());
  printf("\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestPartResult(
    const TestPartResult& result) {
  if (result.type() == TestPartResult::kSuccess)
    return;

  PrintTestPartResult(result);
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
  if (test_info.result()->Passed()) {
    ColoredPrintf(COLOR_GREEN, "[       OK ] ");
  } else {
    ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
  }
  PrintTestName(test_info.test_case_name(), test_info.name());
  if (test_info.result()->Failed())
    PrintFullTestCommentIfPresent(test_info);

  printf("\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestCaseEnd(const TestCase& test_case) {
  const std::string counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
      ColoredPrintf(COLOR_GREEN, "[----------] ");
      printf("%s from %s\n\n", counts.c_str(), test_case.name());
      fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnEnvironmentsTearDownStart(const UnitTest&) {
  ColoredPrintf(COLOR_GREEN, "[----------] ");
  printf("Global test environment tear-down");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::PrintFailedTests(const UnitTest& unit_test) {
  const int failed_test_count = unit_test.failed_test_count();
  if (failed_test_count == 0) {
    return;
  }

  for (int i = 0; i < unit_test.total_test_case_count(); ++i) {
    const TestCase& test_case = *unit_test.GetTestCase(i);
    if (test_case.failed_test_count() == 0) {
      continue;
    }
    for (int j = 0; j < test_case.total_test_count(); ++j) {
      const TestInfo& test_info = *test_case.GetTestInfo(j);
      if (test_info.result()->Passed()) {
        continue;
      }
      ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
      printf("%s.%s", test_case.name(), test_info.name());
      PrintFullTestCommentIfPresent(test_info);
      printf("\n");
    }
  }
}

void PrettyUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                     int iteration) {
  ColoredPrintf(COLOR_GREEN, "[==========] ");
  printf("%s from %s ran.",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
  // if ()
  printf("\n");
  ColoredPrintf(COLOR_GREEN, "[  PASSED  ] ");
  printf("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

  int num_failures = unit_test.failed_test_count();
  if (unit_test.Passed()) {
    const int failed_test_count = unit_test.failed_test_count();
    ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
    printf("%s, listed below:\n", FormatTestCount(failed_test_count).c_str());
    PrintFailedTests(unit_test);
    printf("\n%2d FAILED %s\n", num_failures,
                        num_failures == 1 ? "TEST" : "TESTS");
  }

  // int 
  fflush(stdout);
}

DefaultGlobalTestPartResultReporter::
  DefaultGlobalTestPartResultReporter(
    UnitTest* unit_test) : unit_test_(unit_test) {}

void DefaultGlobalTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->current_test_result()->AddTestPartResult(result);
  unit_test_->listeners().repeater()->OnTestPartResult(result);
}

DefaultPerThreadTestPartResultReporter::
  DefaultPerThreadTestPartResultReporter(
    UnitTest* unit_test) : unit_test_(unit_test) {}

void DefaultPerThreadTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->GetGlobalTestPartResultReporter()->ReportTestPartResult(result);
}

} // namespace internal

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

bool TestResult::Failed() const {
  for (int i = 0; i < total_part_count(); ++i) {
    if (GetTestPartResult(i).failed())
      return true;
  }
  return false;
}

const TestPartResult& TestResult::GetTestPartResult(int i) const {
  if (i < 0 || i >= total_part_count())
    internal::posix::Abort();
  return test_part_results_.at(i);
}

int TestResult::total_part_count() const {
  return static_cast<int>(test_part_results_.size());
}

void TestResult::AddTestPartResult(const TestPartResult& test_part_result) {
  test_part_results_.push_back(test_part_result);
}

TestInfo::TestInfo(const std::string& test_case_name,
                   const std::string& name,
                   internal::CodeLocation a_code_location,
                   internal::TestFactoryBase* factory)
    : test_case_name_(test_case_name),
      name_(name),
      location_(a_code_location),
      factory_(factory) {}

TestInfo::~TestInfo() {
  delete factory_;
}

void TestInfo::Run() {
  UnitTest* unit_test = UnitTest::GetInstance();
  unit_test->set_current_test_info(this);
  TestEventListener* repeater = unit_test->listeners().repeater();

  repeater->OnTestStart(*this);

  Test* test = factory_->CreateTest();
  if (test != NULL) {
    test->Run();
    delete test;
  }

  repeater->OnTestEnd(*this);

  unit_test->set_current_test_info(NULL);
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
  test_indices_.push_back(static_cast<int>(test_indices_.size()));
}

void TestCase::Run() {
  for (auto it = test_info_list_.begin();
       it != test_info_list_.end();
       ++it) {
    (*it)->Run();
  }
}

const TestInfo* TestCase::GetTestInfo(int i) const {
  const int index = GetElementOr(test_indices_, i, -1);
  return index < 0 ? NULL : test_info_list_[index];
}

int TestCase::successful_test_count() const {
  return CountIf(test_info_list_, TestPassed);
}

int TestCase::failed_test_count() const {
  return CountIf(test_info_list_, TestFailed);
}

int TestCase::total_test_count() const {
  return static_cast<int>(test_info_list_.size());
}

int TestCase::test_to_run_count() const {
  return CountIf(test_info_list_, ShouldRunTest);
}

UnitTest::UnitTest()
    : default_global_test_part_result_reporter_(this),
      global_test_part_result_reporter_(
          &default_global_test_part_result_reporter_),
      default_per_thread_test_part_result_reporter_(this),
      per_thread_test_part_result_reporter_(
          &default_per_thread_test_part_result_reporter_),
      current_test_case_(NULL),
      current_test_info_(NULL) {
  listeners().SetDefaultResultPrinter(new
    internal::PrettyUnitTestResultPrinter);
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
  bool failed = false;
  TestEventListener* repeater = listeners().repeater();
  repeater->OnTestProgramStart(*this);

  for (auto it = test_cases_.begin();
       it != test_cases_.end();
       ++it) {
    (*it)->Run();
  }

  repeater->OnTestProgramEnd(*this);

  if (!Passed()) {
    failed = true;
  }

  return !failed;
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
    test_case_indices_.push_back(static_cast<int>(test_case_indices_.size()));
    return test_case;
  }
}

void UnitTest::AddTestInfo(TestInfo* test_info) {
  GetTestCase(test_info->test_case_name())->AddTestInfo(test_info);
}

void UnitTest::AddTestPartResult(
    TestPartResult::Type result_type,
    const char* file_name,
    int line_number,
    const std::string& message) {
  Message msg;
  msg << message;

  // std::cout << "[" << message << "]" << std::endl;
  internal::MutexLock lock(&mutex_);
  // if ()
  const TestPartResult result = 
    TestPartResult(result_type, file_name, line_number,
                   msg.GetString().c_str());

  GetTestPartResultReporterForCurrentThread()->
      ReportTestPartResult(result);

  // if (result_type != TestPartResult::kSuccess) {

  // }
}

int UnitTest::successful_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::successful_test_count);
}

int UnitTest::failed_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::failed_test_count);
}

int UnitTest::failed_test_case_count() const {
  return CountIf(test_cases_, TestCaseFailed);
}

int UnitTest::total_test_case_count() const {
  return static_cast<int>(test_cases_.size());
}

int UnitTest::test_case_to_run_count() const {
  return CountIf(test_cases_, ShouldRunTestCase);
}

int UnitTest::test_to_run_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::test_to_run_count);
}

const TestCase* UnitTest::GetTestCase(int i) const {
  const int index = GetElementOr(test_case_indices_, i, -1);
  return index < 0 ? NULL : test_cases_[i];
}

bool UnitTest::Passed() const {
  return !Failed();
}

bool UnitTest::Failed() const {
  return failed_test_case_count() > 0;
}

TestResult* UnitTest::current_test_result() {
  return current_test_info_ ? &(current_test_info_->result_) : NULL;
}

TestPartResultReporterInterface*
UnitTest::GetGlobalTestPartResultReporter() {
  internal::MutexLock lock(&global_test_part_result_reporter_mutex_);
  return global_test_part_result_reporter_;
}

TestPartResultReporterInterface*
UnitTest::GetTestPartResultReporterForCurrentThread() {
  return per_thread_test_part_result_reporter_.get();
}

void InitGoogleTest(int* argc, char** argv) {}

namespace internal {

std::string AppendUserMessage(const std::string& gtest_msg,
                              const Message& user_msg) {
  const std::string user_msg_string = user_msg.GetString();
  if (user_msg_string.empty()) {
    return gtest_msg;
  }

  return gtest_msg + "\n" + user_msg_string;
}

TestInfo* MakeAndRegisterTestInfo (
    const char* test_case_name,
    const char* name,
    CodeLocation code_location,
    TestFactoryBase* factory) {
  TestInfo* const test_info =
      new TestInfo(test_case_name, name, code_location, factory);
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
  UnitTest::GetInstance()->
    AddTestPartResult(data_->type, data_->file, data_->line,
                      AppendUserMessage(data_->message, message)
                      // Skips the stack frame for this function itself.
                      );  // NOLINT
}

} // internal
} // testing
