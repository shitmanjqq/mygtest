#ifndef GTEST_DEF_H_
#define GTEST_DEF_H_

#define GTEST_MUST_USE_RESULT_ __attribute__ ((warn_unused_result))
#define GTEST_API_ __attribute__((visibility ("default")))
#define GTEST_DISALLOW_COPY_AND_ASSIGN_(classname) \
  classname(const classname &);\
  void operator=(classname const &)

#endif // GTEST_DEF_H_