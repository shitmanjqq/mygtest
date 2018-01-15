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

// class UnitTest* GetUnitTest();

} // namespace internal

class GTEST_API_ Test {
 public:
  friend class TestInfo;

  typedef internal::SetUpTestCaseFunc SetUpTestCaseFunc;
  typedef internal::TearDownTestCaseFunc TearDownTestCaseFunc;

  virtual ~Test();

 protected:
  Test();

 private:
  virtual void TestBody() = 0;

  void Run();

  GTEST_DISALLOW_COPY_AND_ASSIGN_(Test);
};


class GTEST_API_ TestResult {
 public:
  TestResult();

  ~TestResult();

 private:
  friend class TestInfo;
  friend class TestCase;
  friend class UnitTest;

  void Clear();

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestResult);
};


class GTEST_API_ TestInfo {
 public:
  ~TestInfo();

  const char* test_case_name() const { return test_case_name_.c_str(); }

  const char* name() const { return name_.c_str(); }

 private:
  friend class Test;
  friend class TestCase;

  friend TestInfo* internal::MakeAndRegisterTestInfo (
      const char* test_case_name,
      const char* name,
      internal::TestFactoryBase* factory);

  TestInfo(const std::string& test_case_name,
           const std::string& name,
           internal::TestFactoryBase* factory);

  void Run();

  static void ClearTestResult(TestInfo* test_info) {
    test_info->result_.Clear();
  }

  const std::string test_case_name_;
  const std::string name_;
  internal::TestFactoryBase* const factory_;

  TestResult result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestInfo);
};

class GTEST_API_ TestCase {
 public:
  TestCase(const char *name);

  virtual ~TestCase();

  const char *name() const { return name_.c_str(); }

 private:
  friend class Test;
  friend class UnitTest;

  std::vector<TestInfo*>& test_info_list() { return test_info_list_; }

  const std::vector<TestInfo*>& test_info_list() const { return test_info_list_; }

  void AddTestInfo(TestInfo*);

  void Run();

  const std::string name_;
  std::vector<TestInfo*> test_info_list_;
  // std::vector<

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestCase);
};

class GTEST_API_ UnitTest {
 public:
  static UnitTest* GetInstance();

  int Run() GTEST_MUST_USE_RESULT_;

  TestCase* GetTestCase(const char *test_case_name);

  void AddTestInfo(TestInfo* test_info);

 private:
  friend class Test;

  UnitTest();

  virtual ~UnitTest();

  std::vector<TestCase*> test_cases_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTest);
};

inline class UnitTest* GetUnitTest() {
  return UnitTest::GetInstance();
}

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
