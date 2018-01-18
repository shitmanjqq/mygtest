#ifndef GTEST_H_
#define GTEST_H_

#include <iostream>
#include <string>
#include <vector>

#include "gtest_def.h"
#include "gtest_internal.h"
#include "gtest_message.h"
#include "gtest_pred_impl.h"
#include "gtest_port.h"
#include "gtest_test_part.h"
#include "gtest_printers.h"

namespace testing {

class Test;
class TestInfo;
class TestCase;
class UnitTest;

namespace internal {

class TestEventRepeater;
class DefaultGlobalTestPartResultReporter;
class UnitTestImpl;
struct TraceInfo;
UnitTestImpl* GetUnitTestImpl();

} // namespace internal

class GTEST_API_ AssertionResult {
 public:
  // ~AssertionResult() { std::cout << *message_ << std::endl; }
  AssertionResult(const AssertionResult& other);

  template <typename T,
            typename = typename internal::EnableIf<
                !internal::ImplicitlyConvertible<T, AssertionResult>::
                  value>::type>
  explicit AssertionResult(const T& success)
      : success_(success) {}

  AssertionResult& operator=(AssertionResult& other) {
    swap(other);
    return *this;
  }

  operator bool() const { return success_; }

  AssertionResult operator!() const;

  const char* message() const {
    return message_.get() != NULL ? message_->c_str() : "";
  }

  const char* failure_message() const { return message(); }

  template <typename T>
  AssertionResult& operator<<(const T& value) {
    AppendMessage(Message() << value);
    return *this;
  }

  AssertionResult& operator<<(
      ::std::ostream& (*basic_manipulator)(::std::ostream& stream)) {
    AppendMessage(Message() << basic_manipulator);
    return *this;
  }

 private:
  void AppendMessage(const Message& a_message) {
    if (message_.get() == NULL)
      message_.reset(new ::std::string);
    message_->append(a_message.GetString().c_str());
  }

  void swap(AssertionResult& other);

  bool success_;

  internal::scoped_ptr< ::std::string> message_;
};

GTEST_API_ AssertionResult AssertionSuccess();

GTEST_API_ AssertionResult AssertionFailure();

namespace internal {

class AssertHelper;
// class UnitTest* GetUnitTest();

} // namespace internal

typedef internal::TimeInMillis TimeInMillis;

class GTEST_API_ TestProperty;


/************************************************
 * Test
 ************************************************/
class GTEST_API_ Test {
 public:
  friend class TestInfo;

  typedef internal::SetUpTestCaseFunc SetUpTestCaseFunc;
  typedef internal::TearDownTestCaseFunc TearDownTestCaseFunc;

  virtual ~Test();

  static void SetUpTestCase();

  static void TearDownTestCase();

  static bool HasFatalFailure();

  static bool HasNonfatalFalure();

  static bool HasFailure();

  static void RecordProperty(const std::string& key, const std::string& value);
  static void RecordProperty(const std::string& key, int value);

 protected:
  Test();

  virtual void SetUp() {}

  virtual void TearDown() {}

 private:
  static bool HasSameFixtureClass();

  virtual void TestBody() = 0;

  void Run();

  void DeleteSelf_();

  GTEST_DISALLOW_COPY_AND_ASSIGN_(Test);
};


/************************************************
 * TestResult
 ************************************************/
class TestProperty;


/************************************************
 * TestResult
 ************************************************/
class GTEST_API_ TestResult {
 public:
  TestResult();

  ~TestResult();

  int total_part_count() const;

  int test_property_count() const;

  bool Passed() const { return !Failed(); }

  bool Failed() const;

  bool HasFatalFailure() const;

  bool HasNonfatalFalure() const;

  TimeInMillis elapsed_time() const;

  const TestPartResult& GetTestPartResult(int i) const;

  const TestProperty& GetTestProperty(int i) const;

 private:
  friend class TestInfo;
  friend class TestCase;
  friend class UnitTest;
  friend class internal::DefaultGlobalTestPartResultReporter;
  friend class internal::UnitTestImpl;

  const std::vector<TestPartResult>& test_part_results() const {
    return test_part_results_;
  }

  const std::vector<TestProperty>& test_properties() const;

  void set_elapsed_time(TimeInMillis elapsed);

  void RecordProperty(const std::string& xml_element,
                      const TestProperty& test_property);

  static bool ValidateTestProperty(const std::string& xml_element,
                                   const TestProperty& test_property);

  void AddTestPartResult(const TestPartResult& test_part_result);

  int death_test_count() const;

  int increment_death_test_count();

  void ClearTestPartResults();

  void Clear();

  std::vector<TestPartResult> test_part_results_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestResult);
};


/************************************************
 * TestInfo
 ************************************************/
class GTEST_API_ TestInfo {
 public:
  ~TestInfo();

  const char* test_case_name() const { return test_case_name_.c_str(); }

  const char* name() const { return name_.c_str(); }

  const char* type_param() const;

  const char* value_param() const;

  const char* file() const { return location_.file.c_str(); }

  int line() const { return location_.line; }

  bool should_run() const;

  bool is_reportable() const;

  const TestResult* result() const { return &result_; }

 private:
  friend class Test;
  friend class TestCase;
  friend class UnitTest;
  friend class internal::UnitTestImpl;

  friend TestInfo* internal::MakeAndRegisterTestInfo (
      const char* test_case_name,
      const char* name,
      internal::CodeLocation code_location,
      internal::TestFactoryBase* factory);

  TestInfo(const std::string& test_case_name,
           const std::string& name,
           internal::CodeLocation a_code_location,
           internal::TestFactoryBase* factory);

  int increment_death_test_count();

  void Run();

  static void ClearTestResult(TestInfo* test_info) {
    test_info->result_.Clear();
  }

  const std::string test_case_name_;
  const std::string name_;
  internal::CodeLocation location_;
  internal::TestFactoryBase* const factory_;

  TestResult result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestInfo);
};


/************************************************
 * TestCase
 ************************************************/
class GTEST_API_ TestCase {
 public:
  TestCase(const char *name);

  virtual ~TestCase();

  const char *name() const { return name_.c_str(); }

  const char* type_param() const;

  bool should_run() const { return true; }

  int successful_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int test_to_run_count() const;

  int total_test_count() const;

  bool Passed() const { return !Failed(); }

  bool Failed() const { return failed_test_count() > 0; }

  TimeInMillis elapsed_time() const;

  const TestInfo* GetTestInfo(int i) const;

  const TestResult& ad_hoc_test_result() const;

 private:
  friend class Test;
  friend class UnitTest;
  friend class internal::UnitTestImpl;

  std::vector<TestInfo*>& test_info_list() { return test_info_list_; }

  const std::vector<TestInfo*>& test_info_list() const { return test_info_list_; }

  TestInfo* GetMutableTestInfo(int i);

  void set_should_run(bool should);

  void AddTestInfo(TestInfo* test_info);

  void ClearResult();

  static void ClearTestCaseResult(TestCase* test_case) {
    test_case->ClearResult();
  }

  void Run();

  void RunSetUpTestCase();

  void RunTearDownTestCase();

  static bool TestPassed(const TestInfo* test_info) {
    return test_info->result()->Passed();
  }

  static bool TestFailed(const TestInfo* test_info) {
    return test_info->result()->Failed();
  }

  static bool TestReportableDisabled(const TestInfo* test_info);

  static bool TestDisabled(const TestInfo* test_info);

  static bool TestReportable(const TestInfo* test_info);

  static bool ShouldRunTest(const TestInfo* test_info) {
    // return test_info->should_run();
    return true;
  }

  void ShuffleTests(internal::Random* random);

  void UnshuffleTests();

  const std::string name_;
  std::vector<TestInfo*> test_info_list_;
  std::vector<int> test_indices_;
  // std::vector<

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestCase);
};


/************************************************
 * Environment
 ************************************************/
class Environment;


/************************************************
 * TestEventListener
 ************************************************/
class TestEventListener {
 public:
  virtual ~TestEventListener() {}

  virtual void OnTestProgramStart(const UnitTest& unit_test) = 0;

  virtual void OnTestIterationStart(const UnitTest& unit_test,
                                    int iteration) = 0;

  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test) = 0;

  virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) = 0;

  virtual void OnTestCaseStart(const TestCase& test_case) = 0;

  virtual void OnTestStart(const TestInfo& test_info) = 0;

  virtual void OnTestPartResult(const TestPartResult& test_part_result) = 0;

  virtual void OnTestEnd(const TestInfo& test_info) = 0;

  virtual void OnTestCaseEnd(const TestCase& test_case) = 0;

  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test) = 0;

  virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) = 0;

  virtual void OnTestIterationEnd(const UnitTest& unit_test,
                                  int iteration) = 0;

  virtual void OnTestProgramEnd(const UnitTest& unit_test) = 0;
};


/************************************************
 * EmptyTestEventListener
 ************************************************/
class EmptyTestEventListener : public TestEventListener {
 public:
  virtual void OnTestProgramStart(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationStart(const UnitTest& /*unit_test*/,
                                    int /*iteration*/) {}
  virtual void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) {}
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestCaseStart(const TestCase& /*test_case*/) {}
  virtual void OnTestStart(const TestInfo& /*test_info*/) {}
  virtual void OnTestPartResult(const TestPartResult& /*test_part_result*/) {}
  virtual void OnTestEnd(const TestInfo& /*test_info*/) {}
  virtual void OnTestCaseEnd(const TestCase& /*test_case*/) {}
  virtual void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) {}
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationEnd(const UnitTest& /*unit_test*/,
                                  int /*iteration*/) {}
  virtual void OnTestProgramEnd(const UnitTest& /*unit_test*/) {}
};


/************************************************
 * TestEventListeners
 ************************************************/
class GTEST_API_ TestEventListeners {
 public:
  TestEventListeners();
  ~TestEventListeners();

  void Append(TestEventListener* listener);

  TestEventListener* Release(TestEventListener* listener);

  TestEventListener* default_result_printer() const {
    return default_result_printer_;
  }

  TestEventListener* default_xml_generator() const {
    return default_xml_generator_;
  }

 private:
  friend class UnitTest;
  friend class TestInfo;
  friend class internal::DefaultGlobalTestPartResultReporter;
  friend class TestCase;
  friend class internal::UnitTestImpl;

  TestEventListener* repeater();

  void SetDefaultResultPrinter(TestEventListener* listener);

  void SetDefaultXmlGenerator(TestEventListener* listener);

  bool EventForwardingEnabled() const;
  void SuppressEventForwarding();

  internal::TestEventRepeater* repeater_;
  TestEventListener* default_result_printer_;
  TestEventListener* default_xml_generator_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestEventListeners);
};


/************************************************
 * UnitTest
 ************************************************/
class GTEST_API_ UnitTest {
 public:
  static UnitTest* GetInstance();

  int Run() GTEST_MUST_USE_RESULT_;

  const char* original_working_dir() const;

  const TestCase* current_test_case() const;

  const TestInfo* current_test_info() const;

  int random_seed() const;

  int successful_test_case_count() const;

  int failed_test_case_count() const;

  int total_test_case_count() const;

  int test_case_to_run_count() const;

  int successful_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int total_test_count() const;

  int test_to_run_count() const;

  TimeInMillis start_timestamp() const;

  TimeInMillis elapsed_time() const;

  bool Passed() const;

  bool Failed() const;

  const TestCase* GetTestCase(int i) const;

  const TestResult& ad_hoc_test_result() const;

  // TestCase* GetTestCase(const char *test_case_name);

  // void AddTestInfo(TestInfo* test_info);

  TestEventListeners& listeners();

  // void set_current_test_info(TestInfo* a_current_test_info) {
  //   current_test_info_ = a_current_test_info;
  // }

  // TestResult* current_test_result();

  // TestPartResultReporterInterface* GetGlobalTestPartResultReporter();

  // TestPartResultReporterInterface* GetTestPartResultReporterForCurrentThread();

 private:
  friend class Test;
  friend class internal::AssertHelper;
  friend internal::UnitTestImpl* internal::GetUnitTestImpl();

  Environment* AddEnvironment(Environment* env);

  void AddTestPartResult(TestPartResult::Type result_type,
                         const char* file_name,
                         int line_num,
                         const std::string& message);

  void RecordProperty(const std::string& key, const std::string& value);

  TestCase* GetMutableTestCase(int i);

  internal::UnitTestImpl* impl() { return impl_; }
  const internal::UnitTestImpl* impl() const { return impl_; }

  UnitTest();

  virtual ~UnitTest();

  void PushGTestTrace(const internal::TraceInfo& trace);

  void PopGTestTrace();

  // internal::DefaultGlobalTestPartResultReporter default_global_test_part_result_reporter_;
  // TestPartResultReporterInterface* global_test_part_result_reporter_;
  // internal::DefaultPerThreadTestPartResultReporter
  //     default_per_thread_test_part_result_reporter_;
  // internal::ThreadLocal<TestPartResultReporterInterface*>
  //     per_thread_test_part_result_reporter_;
  // std::vector<TestCase*> test_cases_;
  // std::vector<int> test_case_indices_;
  // TestEventListeners listeners_;

  // TestCase* current_test_case_;
  // TestInfo* current_test_info_;
  // Te
  mutable internal::Mutex mutex_;

  internal::UnitTestImpl* impl_;

  // internal::Mutex global_test_part_result_reporter_mutex_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTest);
};



// inline class UnitTest* GetUnitTest() {
//   return UnitTest::GetInstance();
// }

GTEST_API_ void InitGoogleTest(int* argc, char** argv);

namespace internal {

class GTEST_API_ AssertHelper {
 public:
  AssertHelper(TestPartResult::Type type,
               const char* file,
               int line,
               const char* message);
  ~AssertHelper();

  void operator=(const Message& message) const;

 private:
  struct AssertHelperData {
    AssertHelperData(TestPartResult::Type t,
                     const char* srcfile,
                     int line_num,
                     const char* msg)
        : type(t), file(srcfile), line(line_num), message(msg) {}

    TestPartResult::Type const type;
    const char* const file;
    int const line;
    std::string const message;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(AssertHelperData);
  };

  AssertHelperData* const data_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(AssertHelper);
};

// inline class ::testing::UnitTest* GetUnitTest() {
//   return ::testing::UnitTest::GetInstance();
// }

template <typename T1, typename T2>
AssertionResult CmpHelperEQFailure(const char* lhs_expression,
                                   const char* rhs_expression,
                                   const T1& lhs,
                                   const T2& rhs) {
  return EqFailure(lhs_expression,
                   rhs_expression,
                   FormatForComparisonFailureMessage(lhs, rhs),
                   FormatForComparisonFailureMessage(rhs, lhs),
                   false);
}

template <typename T1, typename T2>
AssertionResult CmpHelperEQ(const char* lhs_expression,
                            const char* rhs_expression,
                            const T1& lhs,
                            const T2& rhs) {
  if (lhs == rhs) {
    return AssertionSuccess();
  }

  return CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

GTEST_API_ AssertionResult CmpHelperEQ(const char* lhs_expression,
                                       const char* rhs_expression,
                                       BiggestInt lhs,
                                       BiggestInt rhs);

#define GTEST_IMPL_CMP_HELPER_(op_name, op) \
template <typename T1, typename T2> \
AssertionResult CmpHelper##op_name(const char* lhs_expression, \
                                   const char* rhs_expression, \
                                   const T1& lhs, \
                                   const T2& rhs) { \
  if (lhs op rhs) { \
    return AssertionSuccess(); \
  } else { \
    return EqFailure(lhs_expression, \
                     rhs_expression, \
                     FormatForComparisonFailureMessage(lhs, rhs), \
                     FormatForComparisonFailureMessage(rhs, lhs), \
                     #op); \
  } \
} \
GTEST_API_ AssertionResult CmpHelper##op_name(const char* lhs_expression, \
                                              const char* rhs_expression, \
                                              BiggestInt lhs, \
                                              BiggestInt rhs)

GTEST_IMPL_CMP_HELPER_(NE, !=);
GTEST_IMPL_CMP_HELPER_(LE, <=);
GTEST_IMPL_CMP_HELPER_(LT, <);
GTEST_IMPL_CMP_HELPER_(GE, >=);
GTEST_IMPL_CMP_HELPER_(GT, >);

template <bool lhs_is_null_literal>
class EqHelper {
 public:
  template <typename T1, typename T2>
  static AssertionResult Compare(const char* lhs_expression,
                                 const char* rhs_expression,
                                 const T1& lhs,
                                 const T2& rhs) {
    return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
  }

  // this overload version is for the bug of gcc-4.0
  static AssertionResult Compare(const char* lhs_expression,
                                 const char* rhs_expression,
                                 BiggestInt lhs,
                                 BiggestInt rhs) {
    return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
  }
};

template <>
class EqHelper<true> {
 public:
  template <typename T1, typename T2>
  static AssertionResult Compare(
      const char* lhs_expression,
      const char* rhs_expression,
      const T1& lhs,
      const T2& rhs,
      typename EnableIf<!is_pointer<T2>::value>::type* = 0) {
    return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
  }

  template <typename T>
  static AssertionResult Compare(
      const char* lhs_expression,
      const char* rhs_expression,
      Secret*,
      T* rhs) {
    return CmpHelperEQ(lhs_expression, rhs_expression,
                       static_cast<T*>(NULL), rhs);
  }
};

// inline class ::testing::UnitTest* GetUnitTest() {
//   return ::testing::UnitTest::GetInstance();
// }

} // namespace internal
} // namespace testing

#define EXPECT_EQ(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal:: \
                      EqHelper<GTEST_IS_NULL_LITERAL_(val1)>::Compare, \
                      val1, val2)


#define EXPECT_NE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperNE, val1, val2)

#define EXPECT_LE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperLE, val1, val2)

#define EXPECT_LT(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperLT, val1, val2)

#define EXPECT_GE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperGE, val1, val2)

#define EXPECT_GT(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperGT, val1, val2)

#define GTEST_ASSERT_EQ(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal:: \
                      EqHelper<GTEST_IS_NULL_LITERAL_(val1)>::Compare, \
                      val1, val2)

#define GTEST_TEST_CLASS_NAME_(test_case, test_name)\
  test_case##_##test_name##_Test

#define GTEST_TEST_(test_case, test_name, parent) \
class GTEST_API_ GTEST_TEST_CLASS_NAME_(test_case, test_name) : public parent {\
 public:\
  GTEST_TEST_CLASS_NAME_(test_case, test_name)() {}\
 private:\
  virtual void TestBody();\
  static testing::TestInfo* test_info_;\
  GTEST_DISALLOW_COPY_AND_ASSIGN_(GTEST_TEST_CLASS_NAME_(test_case, test_name));\
};\
\
testing::TestInfo* GTEST_TEST_CLASS_NAME_(test_case, test_name)\
  ::test_info_ = testing::internal::MakeAndRegisterTestInfo(\
      #test_case, #test_name,\
      ::testing::internal::CodeLocation(__FILE__, __LINE__), \
      new testing::internal::TestFactoryImpl<\
          GTEST_TEST_CLASS_NAME_(test_case, test_name)>);\
\
void GTEST_TEST_CLASS_NAME_(test_case, test_name)::TestBody()

#define GTEST_TEST(test_case, test_name) \
  GTEST_TEST_(test_case, test_name,\
              ::testing::Test)

#define TEST(test_case, test_name) \
  GTEST_TEST(test_case, test_name)

#define TEST_F(test_case, test_name) \
  GTEST_TEST_(test_case, test_name, test_case)

inline int RUN_ALL_TESTS() {
  return testing::UnitTest::GetInstance()->Run();
}

#endif // GTEST_H_
