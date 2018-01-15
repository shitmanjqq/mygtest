#ifndef GTEST_TEST_PART_H_
#define GTEST_TEST_PART_H_

#include <string>

#include "gtest_def.h"

namespace testing {

class GTEST_API_ TestPartResult {
 public:
  enum Type {
    kSuccess,
    kNonFatalFailure,
    kFatalFailure,
  };

  TestPartResult(Type a_type,
                 const char* a_file_name,
                 int a_line_number,
                 const char* a_message)
      : type_(a_type),
        file_name_(a_file_name == NULL ? "" : a_file_name),
        line_number_(a_line_number),
        summary_(ExtractSummary(a_message)),
        message_(a_message) {
  }

  Type type() const { return type_; }

  const char* file_name() const {
    return file_name_.empty() ? NULL : file_name_.c_str();
  }

  int line_number() const { return line_number_; }

  const char* summary() const { return summary_.c_str(); }

  const char* message() const { return message_.c_str(); }

  bool passed() const { return type_ == kSuccess; }

  bool failed() const { return type_ != kSuccess; }

  bool nonfatally_failed() const { return type_ == kNonFatalFailure; }

  bool fatally_failed() const { return type_ == kFatalFailure; }

 private:
  static std::string ExtractSummary(const char* message);

  Type type_;
  std::string file_name_;
  int line_number_;
  std::string summary_;
  std::string message_;
};

class TestPartResultReporterInterface {
 public:
  virtual ~TestPartResultReporterInterface() {}

  virtual void ReportTestPartResult(const TestPartResult& result) = 0;
};

}

#endif