#include <iostream>

#include "gtest.h"

TEST(MyTest, first) {
  // std::cout << "MyTest: first test" << std::endl;
  EXPECT_EQ(1, 3);
}

TEST(MyTest, second) {
  // std::cout << "MyTest: second test" << std::endl;
}

TEST(Hello, first) {
  // std::cout << "Hello: first test" << std::endl;
}

TEST(Hello, second) {
  // std::cout << "Hello: second test" << std::endl;
}

class World : public testing::Test {

};

TEST_F(World, first) {
  // std::cout << "World: first test" << std::endl;
}

TEST_F(World, second) {
  // std::cout << "World: second test" << std::endl;
  EXPECT_EQ(1, 2);
}
