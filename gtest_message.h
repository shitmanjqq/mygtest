#ifndef GTEST_MESSAGE_H_
#define GTEST_MESSAGE_H_

#include <iostream>
#include <sstream>

#include "gtest_def.h"
#include "gtest_port.h"

void operator<<(const testing::internal::Secret&, int);

namespace testing {

class GTEST_API_ Message {
 private:
  typedef std::ostream& (*BasicNarrowIoManip)(std::ostream&);

 public:
  Message();

  Message(const Message& msg) : ss_(new ::std::stringstream) {
    // *ss_ << msg.GetString();
    *ss_ << msg.ss_->str(); // for some optimization
  }

  explicit Message(const char* str) : ss_(new ::std::stringstream) {
    *ss_ << str;
  }

  template <typename T>
  inline Message& operator<<(const T& val) {
    using ::operator <<; // don't know why
    *ss_ << val;
    return *this;
  }

  template <typename T>
  inline Message& operator<<(T* const& pointer) {
    if (NULL == pointer) {
      *ss_ << "(null)";
    } else {
      *ss_ << pointer;
    }
    return *this;
  }

  Message& operator<<(BasicNarrowIoManip val) {
    *ss_ << val;
    return *this;
  }

  Message& operator<<(bool b) {
    return *this << (b ? "true" : "false");
  }

  std::string GetString() const;

 private:
  const internal::scoped_ptr< ::std::stringstream> ss_;

  void operator=(const Message&);
};

inline std::ostream& operator<<(std::ostream& os, const Message& sb) {
  return os << sb.GetString();
}

namespace internal {

template <typename T>
std::string StreamableToString(const T& stream) {
  return (Message() << stream).GetString();
}

} // namespace internal
} // namespace testing

#endif