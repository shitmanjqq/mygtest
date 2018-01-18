#ifndef GTEST_INTERNAL_H_
#define GTEST_INTERNAL_H_

#include <string>
#include <algorithm>

#include "gtest_def.h"
#include "gtest_port.h"
#include "gtest_message.h"
#include "gtest_test_part.h"

namespace testing {

class AssertionResult;
class Test;
class TestInfo;
class UnitTest;
class TestCase;

template <typename T>
::std::string PrintToString(const T& value);

namespace internal {

struct CodeLocation {
  CodeLocation(const std::string& a_file, int a_line)
      : file(a_file), line(a_line) {}

  std::string file;
  int line;
};

GTEST_API_ extern const char kStackTraceMarker[];

GTEST_API_ std::string AppendUserMessage(
    const std::string& gtest_msg, const Message& user_msg);

template <typename Container, typename Predicate>
inline int CountIf(const Container& c, Predicate predicate) {
  int count = 0;
  for (typename Container::const_iterator it = c.begin();
       it != c.end(); ++it) {
    if (predicate(*it))
      ++count;
  }

  return count;
}

template <typename Container, typename Functor>
void ForEach(const Container& c, Functor functor) {
  std::for_each(c.begin(), c.end(), functor);
}



template <typename T>
static void Delete(T* x) {
  delete x;
}

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

TestInfo* MakeAndRegisterTestInfo (
      const char* test_case_name,
      const char* name,
      CodeLocation code_location,
      TestFactoryBase* factory);








/************************************************
 * Random
 ************************************************/
class GTEST_API_ Random;




} // namespace internal
} // namespace testing

#endif // GTEST_INTERNAL_H_
