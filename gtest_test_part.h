#ifndef GTEST_TEST_PART_H_
#define GTEST_TEST_PART_H_

#include "gtest_def.h"

namespace testing {

class GTEST_API_ TestPartResult {
 public:
  enum Type {
    kSuccess,
    kNonFatalFailure,
    kFatalFailure,
  };
};

}

#endif