#ifndef GTEST_PRINTERS_H_
#define GTEST_PRINTERS_H_

#include <string>
#include <sstream>
#include <iostream>

#include "gtest_port.h"

namespace testing {
namespace internal2 {

} // namespace internal2
} // namespace testing

namespace testing_internal {

template <typename T>
void DefaultPrintNonContainerTo(const T& value, ::std::ostream* os) {
  using namespace ::testing::internal2;
  *os << value;
}

} // namespace testing_internal

namespace testing {
namespace internal {

template <typename T>
class UniversalPrinter;

template <typename T>
void UniversalPrint(const T& value, ::std::ostream* os);

enum DefaultPrinterType {
  kPrintContainer,
  kPrintPointer,
  kPrintFunctionPointer,
  kPrintOther,
};
template <DefaultPrinterType type> struct WrapPrinterType {};

template <typename T>
void DefaultPrintTo(WrapPrinterType<kPrintContainer>,
                    const T& container, ::std::ostream* os) {
  const size_t kMaxCount = 32;
  *os << '{';
  size_t count = 0;
  for (typename T::const_iterator it = container.begin();
       it != container.end(); ++it, ++count) {
    if (count > 0) {
      *os << ',';
      if (count == kMaxCount) {
        *os << " ...";
        break;
      }
      *os << ' ';
      internal::UniversalPrint(*it, os);
    }
  }

  if (count > 0) {
    *os << ' ';
  }
  *os << '}';
}

template <typename T>
void DefaultPrintTo(WrapPrinterType<kPrintPointer>,
                    T* p, ::std::ostream* os) {
  if (p == NULL) {
    *os << "NULL";
  } else {
    *os << p;
  }
}

template <typename T>
void DefaultPrintTo(WrapPrinterType<kPrintFunctionPointer>,
                    T* p, ::std::ostream* os) {
  if (p == NULL) {
    *os << "NULL";
  } else {
    *os << reinterpret_cast<const void*>(p);
  }
}

template <typename T>
void DefaultPrintTo(WrapPrinterType<kPrintOther>,
                    const T& value, ::std::ostream* os) {
  ::testing_internal::DefaultPrintNonContainerTo(value, os);
}

template <typename T>
void PrintTo(const T& value, ::std::ostream* os) {
  DefaultPrintTo(
      WrapPrinterType<
          (sizeof(IsContainerTest<T>(0)) == sizeof(IsContainer)) && !IsRecursiveContainer<T>::value
            ? kPrintContainer : !is_pointer<T>::value
              ? kPrintOther
                : !internal::ImplicitlyConvertible<T, const void*>::value
                  ? kPrintFunctionPointer
                  : kPrintPointer>(),
      value, os);
}

GTEST_API_ void PrintTo(unsigned char c, ::std::ostream* os);
GTEST_API_ void PrintTo(signed char c, ::std::ostream* os);
inline void PrintTo(char c, ::std::ostream* os) {
  // When printing a plain char, we always treat it as unsigned.  This
  // way, the output won't be affected by whether the compiler thinks
  // char is signed or not.
  PrintTo(static_cast<unsigned char>(c), os);
}

// Overloads for other simple built-in types.
inline void PrintTo(bool x, ::std::ostream* os) {
  *os << (x ? "true" : "false");
}

// Overload for wchar_t type.
// Prints a wchar_t as a symbol if it is printable or as its internal
// code otherwise and also as its decimal code (except for L'\0').
// The L'\0' char is printed as "L'\\0'". The decimal code is printed
// as signed integer when wchar_t is implemented by the compiler
// as a signed type and is printed as an unsigned integer when wchar_t
// is implemented as an unsigned type.
GTEST_API_ void PrintTo(wchar_t wc, ::std::ostream* os);

// Overloads for C strings.
GTEST_API_ void PrintTo(const char* s, ::std::ostream* os);
inline void PrintTo(char* s, ::std::ostream* os) {
  PrintTo(ImplicitCast_<const char*>(s), os);
}

// signed/unsigned char is often used for representing binary data, so
// we print pointers to it as void* to be safe.
inline void PrintTo(const signed char* s, ::std::ostream* os) {
  PrintTo(ImplicitCast_<const void*>(s), os);
}
inline void PrintTo(signed char* s, ::std::ostream* os) {
  PrintTo(ImplicitCast_<const void*>(s), os);
}
inline void PrintTo(const unsigned char* s, ::std::ostream* os) {
  PrintTo(ImplicitCast_<const void*>(s), os);
}
inline void PrintTo(unsigned char* s, ::std::ostream* os) {
  PrintTo(ImplicitCast_<const void*>(s), os);
}

GTEST_API_ void PrintStringTo(const ::std::string&s, ::std::ostream* os);
inline void PrintTo(const ::std::string& s, ::std::ostream* os) {
  PrintStringTo(s, os);
}

template <typename T>
class UniversalPrinter {
 public:
  static void Print(const T&value, ::std::ostream* os) {
    PrintTo(value, os);
  }
};

template <typename T>
void PrintRawArrayTo(const T a[], size_t count, ::std::ostream* os) {
  UniversalPrint(a[0], os);
  for (size_t i = 1; i != count; i++) {
    *os << ", ";
    UniversalPrint(a[i], os);
  }
}

template <typename T>
void UniversalPrintArray(const T* begin, size_t len, ::std::ostream* os) {
  if (len == 0) {
    *os << "{}";
  } else {
    *os << "{ ";
    const size_t kThreshold = 18;
    const size_t kChunkSize = 8;
    if (len <= kThreshold) {
      PrintRawArrayTo(begin, len, os);
    } else {
      PrintRawArrayTo(begin, kChunkSize, os);
      *os << ", ..., ";
      PrintRawArrayTo(begin + len - kChunkSize, kChunkSize, os);
    }
    *os << " }";
  }
}

GTEST_API_ void UniversalPrintArray(
    const char* begin, size_t len, ::std::ostream* os);


template <typename T, size_t N>
class UniversalPrinter<T[N]> {
 public:
  static void Print(const T (&a)[N], ::std::ostream* os) {
    UniversalPrintArray(a, N, os);
  }
};

template <typename T>
class UniversalTersePrinter {
 public:
  static void Print(const T& value, ::std::ostream* os) {
    UniversalPrint(value, os);
  }
};

template <typename T>
class UniversalTersePrinter<T&> {
 public:
  static void Print(const T& value, ::std::ostream* os) {
    UniversalPrint(value, os);
  }
};

template <typename T, size_t N>
class UniversalTersePrinter<T[N]> {
 public:
  static void Print(const T (&value)[N], ::std::ostream* os) {
    UniversalPrinter<T[N]>::Print(value, os);
  }
};

template <>
class UniversalTersePrinter<const char*> {
 public:
  static void Print(const char* str, ::std::ostream* os) {
    if (str == NULL) {
      *os << "NULL";
    } else {
      UniversalPrint(std::string(str), os);
    }
  }
};

template <>
class UniversalTersePrinter<char*> {
 public:
  static void Print(char* str, ::std::ostream* os) {
    UniversalTersePrinter<const char*>::Print(str, os);
  }
};

template <typename T>
void UniversalPrint(const T& value, ::std::ostream* os) {
  typedef T T1;
  UniversalPrinter<T1>::Print(value, os);
}

template <typename ToPrint, typename OtherOperand>
class FormatForComparison {
 public:
  static ::std::string Format(const ToPrint& value) {
    return ::testing::PrintToString(value);
  }
};

template <typename ToPrint, size_t N, typename OtherOperand>
class FormatForComparison<ToPrint[N], OtherOperand> {
 public:
  static ::std::string Format(const ToPrint* value) {
    return FormatForComparison<const ToPrint*, OtherOperand>::Format(value);
  }
};

template <typename T1, typename T2>
std::string FormatForComparisonFailureMessage(
    const T1& value, const T2&) {
  return FormatForComparison<T1, T2>::Format(value);
}

} // namespace testing

template <typename T>
::std::string PrintToString(const T& value) {
  ::std::stringstream ss;
  internal::UniversalTersePrinter<T>::Print(value, &ss);
  return ss.str();
}

} // namespace internal

#endif