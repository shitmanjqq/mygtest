#ifndef GTEST_PORT_H_
#define GTEST_PORT_H_

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <string.h>
#include <typeinfo>

#include "gtest_def.h"

typedef struct _RTL_CRITICAL_SECTION GTEST_CRITICAL_SECTION;

namespace testing {
namespace internal {

class Secret;

typedef long long BiggestInt;

template <bool bool_value>
struct bool_const {
  typedef bool_const<bool_value> type;
  typedef bool value_type;
  static const bool value = bool_value;
};

typedef bool_const<false> false_type;
typedef bool_const<true> true_type;

template <typename T1, typename T2>
struct is_same : public false_type {};

template <typename T>
struct is_same<T, T> : public true_type {};

template <typename T>
struct is_pointer : public false_type {};

template <typename T>
struct is_pointer<T *> : public true_type {};

template <typename Iterator>
struct IteratorTraits {
  typedef typename Iterator::value_type value_type;
};

template <typename T>
struct IteratorTraits<T*> {
  typedef T value_type;
};

template <typename T>
struct IteratorTraits<const T*> {
  typedef T value_type;
};

GTEST_API_ bool IsTrue(bool condition);

template <typename T>
class scoped_ptr {
 public:
  typedef T element_type;

  explicit scoped_ptr(T *p = NULL) : ptr_(p) {}
  ~scoped_ptr() { reset(); }

  T& operator*() const { return *ptr_; }
  T* operator->() const { return ptr_; }
  T* get() const { return ptr_; }

  T* release() {
    T* const ptr = ptr_;
    ptr_ = NULL;
    return ptr;
  }

  void reset(T* p = NULL) {
    if (p != ptr_) {
      if (IsTrue(sizeof(T) > 0)) {
        delete ptr_;
      }
      ptr_ = p;
    }
  }

  friend void swap(scoped_ptr& a, scoped_ptr& b) {
    using std::swap;
    swap(a.ptr_, b.ptr_);
  }

 private:
  T* ptr_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(scoped_ptr);
};

GTEST_API_ ::std::string FormatFileLocation(const char* file, int line);

enum GTestLogSeverity {
  GTEST_INFO,
  GTEST_WARNING,
  GTEST_ERROR,
  GTEST_FATAL
};

class GTEST_API_ GTestLog {
 public:
  GTestLog(GTestLogSeverity severity, const char* file, int line);

  ~GTestLog();

  ::std::ostream& GetStream() { return ::std::cerr; }

 private:
  const GTestLogSeverity severity_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(GTestLog);
};

#if !defined(GTEST_LOG_)
#define GTEST_LOG_(severity) \
    ::testing::internal::GTestLog(::testing::internal::GTEST_##severity, \
                                  __FILE__, __LINE__).GetStream()

inline void LogToStderr() {}
inline void FlushInfoLog() { fflush(NULL); }
#endif

template<typename To>
inline To ImplicitCast_(To x) { return x; }

#if !defined(GTEST_CHECK_)
#define GTEST_CHECK_(condition) \
  if (::testing::internal::IsTrue(condition)) \
    ; \
  else \
    GTEST_LOG_(FATAL) << "Condition " #condition " failed. "
#endif

#define GTEST_CHECK_POSIX_SUCCESS_(posix_call) \
  if (const int gtest_error = (posix_call)) \
    GTEST_LOG_(FATAL) << #posix_call << "failed with error " \
                      << gtest_error

template <typename Derived, typename Base>
Derived* CheckedDowncastToActualType(Base* base) {
  GTEST_CHECK_(typeid(*base) == typeid(Derived));
  return dynamic_cast<Derived*>(base);
}

class MutexBase {
 public:
  void Lock() {
    GTEST_CHECK_POSIX_SUCCESS_(pthread_mutex_lock(&mutex_));
    owner_ = pthread_self();
    has_owner_ = true;
  }

  void Unlock() {
    has_owner_ = false;
    GTEST_CHECK_POSIX_SUCCESS_(pthread_mutex_unlock(&mutex_));
  }

  void AssertHeld() const {
    GTEST_CHECK_(has_owner_ && pthread_equal(owner_, pthread_self()))
        << "The current thread is not holding the mutex @" << this;
  }

 protected:
  pthread_mutex_t mutex_;
  bool has_owner_;
  pthread_t owner_;
};

class GTEST_API_ Mutex : public MutexBase {
 public:
  Mutex() {
    GTEST_CHECK_POSIX_SUCCESS_(pthread_mutex_init(&mutex_, NULL));
    has_owner_ = false;
  }
  ~Mutex() {
    GTEST_CHECK_POSIX_SUCCESS_(pthread_mutex_destroy(&mutex_));
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(Mutex);
};

class GTestMutexLock {
 public:
  explicit GTestMutexLock(Mutex* mutex)
      : mutex_(mutex) { mutex_->Lock(); }

  ~GTestMutexLock() { mutex_->Unlock(); }

 private:
  MutexBase* const mutex_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(GTestMutexLock);
};

typedef GTestMutexLock MutexLock;

class ThreadLocalValueHolderBase {
 public:
  virtual ~ThreadLocalValueHolderBase() {}
};

extern "C" inline void DeleteThreadLocalValue(void* value_holder) {
  delete static_cast<ThreadLocalValueHolderBase*>(value_holder);
}

template <typename T>
class GTEST_API_ ThreadLocal {
 public:
  ThreadLocal()
      : key_(CreateKey()), default_factory_(new DefaultValueHolderFactory()) {}

  explicit ThreadLocal(const T& value)
      : key_(CreateKey()),
        default_factory_(new InstanceValueHolderFactory(value)) {}

  ~ThreadLocal() {
    DeleteThreadLocalValue(pthread_getspecific(key_));
    GTEST_CHECK_POSIX_SUCCESS_(pthread_key_delete(key_));
  }

  T* pointer() { return GetOrCreateValue(); }
  const T* pointer() const { return GetOrCreateValue(); }
  const T& get() const { return *pointer(); }
  void set(const T& value) { *pointer() = value; }

 private:
  class ValueHolder : public ThreadLocalValueHolderBase {
   public:
    ValueHolder() : value_() {}
    explicit ValueHolder(const T& value) : value_(value) {}

    T* pointer() { return &value_; }

   private:
    T value_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(ValueHolder);
  };

  static pthread_key_t CreateKey() {
    pthread_key_t key;
    GTEST_CHECK_POSIX_SUCCESS_(
        pthread_key_create(&key, &DeleteThreadLocalValue));
    return key;
  }

  T* GetOrCreateValue() const {
    ThreadLocalValueHolderBase* const holder =
        static_cast<ThreadLocalValueHolderBase*>(pthread_getspecific(key_));
    if (holder != NULL) {
      return CheckedDowncastToActualType<ValueHolder>(holder)->pointer();
    }

    ValueHolder* const new_holder = default_factory_->MakeNewHolder();
    ThreadLocalValueHolderBase* const holder_base = new_holder;
    GTEST_CHECK_POSIX_SUCCESS_(pthread_setspecific(key_, holder_base));
    return new_holder->pointer();
  }

  class ValueHolderFactory {
   public:
    ValueHolderFactory() {}
    virtual ~ValueHolderFactory() {}
    virtual ValueHolder* MakeNewHolder() const = 0;

   private:
    GTEST_DISALLOW_COPY_AND_ASSIGN_(ValueHolderFactory);
  };

  class DefaultValueHolderFactory : public ValueHolderFactory {
   public:
    DefaultValueHolderFactory() {}
    virtual ValueHolder* MakeNewHolder() const {
      return new ValueHolder();
    }

   private:
    GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultValueHolderFactory);
  };

  class InstanceValueHolderFactory : public ValueHolderFactory {
   public:
    explicit InstanceValueHolderFactory(const T& value)
        : value_(value) {}

    virtual ValueHolder* MakeNewHolder() const {
      return new ValueHolder(value_);
    }

   private:
    const T value_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(InstanceValueHolderFactory);
  };

  const pthread_key_t key_;
  scoped_ptr<ValueHolderFactory> default_factory_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ThreadLocal);
};

namespace posix {

typedef struct stat StatStruct;

inline int FileNo(FILE* file) { return fileno(file); }
inline int IsATTY(int fd) { return isatty(fd); }
inline int Stat(const char* path, StatStruct* buf) { return stat(path, buf); }
inline int StrCaseCmp(const char* s1, const char* s2) {
  return strcasecmp(s1, s2);
}
inline char* StrDup(const char* src) { return strdup(src); }
inline int RmDir(const char* dir) { return rmdir(dir); }
inline bool IsDir(const StatStruct& st) { return S_ISDIR(st.st_mode); }

inline void Abort() { abort(); }

} // namespace posix

template <size_t size>
class TypeWithSize {
 public:
  typedef void UInt;
};

template <>
class TypeWithSize<4> {
 public:
  typedef int Int;
  typedef unsigned int UInt;
};

template <>
class TypeWithSize<8> {
 public:
  typedef long long Int;
  typedef unsigned long long UInt;
};

typedef TypeWithSize<4>::Int Int32;
typedef TypeWithSize<4>::UInt UInt32;
typedef TypeWithSize<8>::Int Int64;
typedef TypeWithSize<8>::UInt UInt64;
typedef TypeWithSize<8>::Int TimeInMillis;  // Represents time in milliseconds.

#if !defined(GTEST_FLAG)
#define GTEST_FLAG(name) FLAGS_gtest_##name
#endif

#define GTEST_DECLARE_bool_(name) GTEST_API_ extern bool GTEST_FLAG(name)
#define GTEST_DECLARE_int32_(name) \
    GTEST_API_ extern ::testing::internal::Int32 GTEST_FLAG(name)
#define GTEST_DECLARE_string_(name) \
    GTEST_API_ extern ::std::string GTEST_FLAG(name)

#define GTEST_DEFINE_bool_(name, default_val, doc) \
    GTEST_API_ bool GTEST_FLAG(name) = (default_val)
#define GTEST_DEFINE_int32_(name, default_val, doc) \
    GTEST_API_ ::testing::internal::Int32 GTEST_FLAG(name) = (default_val)
#define GTEST_DEFINE_string_(name, default_val, doc) \
    GTEST_API_ ::std::string GTEST_FLAG(name) = (default_val)

bool BoolFromGTestEnv(const char* flag, bool default_val);
GTEST_API_ Int32 Int32FromGTestEnv(const char* flag, Int32 default_val);
std::string StringFromGTestEnv(const char* flag, const char* default_val);

} // namespace internal

} // namespace testing
#endif