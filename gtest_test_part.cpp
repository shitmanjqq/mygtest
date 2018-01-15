#include <cstring>

#include "gtest_internal.h"
#include "gtest_test_part.h"

namespace testing {
namespace internal {

} // namespace internal

std::string TestPartResult::ExtractSummary(const char* message) {
  const char* const stack_trace = strstr(message, internal::kStackTraceMarker);
  return stack_trace == NULL ? message :
      std::string(message, stack_trace);
}

} // namespace testing