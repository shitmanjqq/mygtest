#ifndef GTEST_INTERNAL_H_
#define GTEST_INTERNAL_H_

#include "gtest_def.h"

namespace testing {

class Test;
class TestInfo;

namespace internal {

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

TestInfo* MakeAndRegisterTestInfo (
      const char* test_case_name,
      const char* name,
      TestFactoryBase* factory);

} // namespace internal
} // namespace testing

#endif // GTEST_INTERNAL_H_