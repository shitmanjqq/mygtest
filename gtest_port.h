#ifndef GTEST_PORT_H_
#define GTEST_PORT_H_

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

template<typename To>
inline To ImplicitCast_(To x) { return x; }

} // namespace internal

} // namespace testing
#endif