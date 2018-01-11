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


} // namespace internal


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

#endif // GTEST_H_