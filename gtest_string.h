#ifndef GTEST_STRING_H_
#define GTEST_STRING_H_

#include <string>

#include "gtest_def.h"

namespace testing {
namespace internal {

GTEST_API_ std::string StringStreamToString(::std::stringstream* stream);

} // namespace internal
} // namespace testing

#endif