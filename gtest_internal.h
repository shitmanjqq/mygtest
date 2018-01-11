#ifndef GTEST_INTERNAL_H_
#define GTEST_INTERNAL_H_

#include <string>

#include "gtest_def.h"
#include "gtest_port.h"

namespace testing {

class AssertionResult;
class Test;
class TestInfo;

template <typename T>
::std::string PrintToString(const T& value);

namespace internal {

template <typename T>
struct AddReference {
  typedef T& type;
};

template <typename T>
struct AddReference<T&> {
  typedef T& type;
};

template <typename From, typename To>
class ImplicitlyConvertible {
 private:
  static typename AddReference<From>::type MakeRefer();

  static char test(To);
  static char (&test(...))[2];

 public:
  static bool const value =
      sizeof(test(MakeRefer())) == sizeof(char);
};

char IsNullLiteralHelper(Secret*);
char (&IsNullLiteralHelper(...))[2];

#define GTEST_IS_NULL_LITERAL_(x) \
    (sizeof(::testing::internal::IsNullLiteralHelper(x)) == 1)

template <bool> struct EnableIf;
template<> struct EnableIf<true> { typedef void type; };

typedef void (*SetUpTestCaseFunc)();
typedef void (*TearDownTestCaseFunc)();

class GTEST_API_ TestFactoryBase {
 public:
  virtual ~TestFactoryBase() {}

  virtual Test* CreateTest() = 0;

 protected:
  TestFactoryBase() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestFactoryBase);
};

template <typename TestType>
class GTEST_API_ TestFactoryImpl : public TestFactoryBase {
  virtual Test* CreateTest() {
    return new TestType;
  }
};

GTEST_API_ AssertionResult EqFailure(const char* expected_expression,
                                     const char* actual_expression,
                                     const std::string& expected_value,
                                     const std::string& actual_value,
                                     bool ignoring_case);

typedef int IsContainer;
template <typename T>
IsContainer IsContainerTest(int,
                            typename T::iterator = NULL,
                            typename T::const_iterator = NULL) {
  return 0;
}

typedef char IsNotContainer;
template <typename T>
IsNotContainer IsContainerTest(long) { return '\0'; }

template <typename T, bool =
  sizeof(IsContainerTest<T>(0)) == sizeof(IsContainer)
>
struct IsRecursiveContainerImpl;

template <typename T>
struct IsRecursiveContainerImpl<T, false> : public false_type {};

template <typename T>
struct IsRecursiveContainerImpl<T, true> {
  typedef
    typename IteratorTraits<typename T::iterator>::value_type
  value_type;
  typedef is_same<value_type, T> type;
};

template <typename T>
struct IsRecursiveContainer : public IsRecursiveContainerImpl<T>::type {};

GTEST_API_ AssertionResult EqFailure(const char* expected_expression,
                                     const char* actual_expression,
                                     const std::string& expected_value,
                                     const std::string& actual_value,
                                     bool ignoring_case);

} // namespace internal

} // namespace testing

#endif // GTEST_INTERNAL_H_