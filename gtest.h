#ifndef GTEST_H_
#define GTEST_H_

#include <string>
#include <vector>

#include "gtest_def.h"
#include "gtest_internal.h"

namespace testing {

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

// inline class ::testing::UnitTest* GetUnitTest() {
//   return ::testing::UnitTest::GetInstance();
// }

} // namespace internal
} // namespace testing

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