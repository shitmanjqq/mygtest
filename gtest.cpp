#include <cstring>

#include "gtest.h"

// TODO

namespace testing {

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


} // internal
} // testing