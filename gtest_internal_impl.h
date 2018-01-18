#ifndef GTEST_INTERNAL_IMPL_H_
#define GTEST_INTERNAL_IMPL_H_

#include "gtest.h"

namespace testing {
namespace internal {

template <typename T>
inline T GetElementOr(const std::vector<T>& v, int i, T default_value) {
  return (i < 0 || i >= static_cast<int>(v.size())) ? default_value : v[i];
}

/************************************************
 * OsStackTraceGetterInterface
 ************************************************/
class OsStackTraceGetterInterface;


/************************************************
 * TraceInfo
 ************************************************/
struct TraceInfo {
  const char* file;
  int line;
  std::string message;
};


/************************************************
 * DefaultPerThreadTestPartResultReporter
 ************************************************/
class DefaultPerThreadTestPartResultReporter
    : public TestPartResultReporterInterface {
 public:
  explicit DefaultPerThreadTestPartResultReporter(UnitTestImpl* unit_test);

  virtual void ReportTestPartResult(const TestPartResult& result);

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultPerThreadTestPartResultReporter);
};


/************************************************
 * DefaultGlobalTestPartResultReporter
 ************************************************/
class DefaultGlobalTestPartResultReporter
  : public TestPartResultReporterInterface {
 public:
  explicit DefaultGlobalTestPartResultReporter(UnitTestImpl* unit_test);

  virtual void ReportTestPartResult(const TestPartResult& result);

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultGlobalTestPartResultReporter);
};


/************************************************
 * UnitTestImpl
 ************************************************/
class GTEST_API_ UnitTestImpl {
 public:
  explicit UnitTestImpl(UnitTest* parent);
  virtual ~UnitTestImpl();

  TestPartResultReporterInterface* GetGlobalTestPartResultReporter();

  void SetGlobalTestPartResultReporter(
      TestPartResultReporterInterface* reporter);

  TestPartResultReporterInterface* GetTestPartResultReporterForCurrentThread();

  void SetTestPartResultReporterForCurrentThread(
      TestPartResultReporterInterface* reporter);

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

  bool Passed() const { return !Failed(); }

  bool Failed() const {
    return failed_test_case_count() > 0;
  }

  const TestCase* GetTestCase(int i) const {
    const int index = GetElementOr(test_case_indices_, i, -1);
    return index < 0 ? NULL : test_cases_[i];
  }

  TestCase* GetMutableTestCase(int i) {
    const int index = GetElementOr(test_case_indices_, i, -1);
    return index < 0 ? NULL : test_cases_[index];
  }

  TestEventListeners* listeners() { return &listeners_; }

  TestResult* current_test_result();

  const TestResult* ad_hoc_test_result() const;

  void set_os_stack_trace_getter(OsStackTraceGetterInterface* getter);

  OsStackTraceGetterInterface* os_stack_trace_getter();

  std::string CurrentOsStackTraceExceptTop(int skip_count);

  TestCase* GetTestCase(const char* test_case_name);

  void AddTestInfo(TestInfo* test_info) {
    GetTestCase(test_info->test_case_name())->AddTestInfo(test_info);
  }

  void set_current_test_case(TestCase* a_current_test_case) {
    current_test_case_ = a_current_test_case;
  }

  void set_current_test_info(TestInfo* a_current_test_info) {
    current_test_info_ = a_current_test_info;
  }

  void RegisterParameterizedTests();

  bool RunAllTests();

  void ClearNonAdHocTestResult() {
    ForEach(test_cases_, TestCase::ClearTestCaseResult);
  }

  void ClearAdHocTestResult();

  void RecordProperty(const TestProperty& test_property);

  enum ReactionToSharding {
    HONOR_SHARDING_PROTOCOL,
    IGNORE_SHARDING_PROTOCOL
  };

  int FilterTests(ReactionToSharding shared_tests);

  void ListTestsMatchingFilter();

  const TestCase* current_test_case() const { return current_test_case_; }
  TestInfo* current_test_info() { return current_test_info_; }
  const TestInfo* current_test_info() const { return current_test_info_; }

  std::vector<Environment*>& environments();

  std::vector<TraceInfo>& gtest_trace_stack();
  const std::vector<TraceInfo>& gtest_trace_stack() const;

  void PostFlagParsingInit();

  int random_seed() const;

  internal::Random* random();

  void ShuffleTests();

  void UnshuffleTests();

  bool catch_exceptions() const;

 private:
  friend class ::testing::UnitTest;

  void set_catch_exceptions(bool value);

  UnitTest* const parent_;

  DefaultGlobalTestPartResultReporter default_global_test_part_result_reporter_;
  DefaultPerThreadTestPartResultReporter
      default_per_thread_test_part_result_reporter_;

  TestPartResultReporterInterface* global_test_part_result_repoter_;

  internal::Mutex global_test_part_result_reporter_mutex_;

  internal::ThreadLocal<TestPartResultReporterInterface*>
      per_thread_test_part_result_reporter_;

  std::vector<TestCase*> test_cases_;
  std::vector<int> test_case_indices_;

  TestCase* current_test_case_;
  TestInfo* current_test_info_;

  TestEventListeners listeners_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTestImpl);
};

inline UnitTestImpl* GetUnitTestImpl() {
  return UnitTest::GetInstance()->impl();
}

}
}

#endif