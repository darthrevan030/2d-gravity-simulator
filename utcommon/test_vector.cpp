#include "vector.hpp"

#include <gtest/gtest.h>

TEST(test_vector, default_constructor) {
  common::vec2d const v;
  EXPECT_EQ(v.getx(), 0.0);
  EXPECT_EQ(v.gety(), 0.0);
}

TEST(test_vector, value_constructor) {
  common::vec2d const v{1.5, -2.5};
  EXPECT_EQ(v.getx(), 1.5);
  EXPECT_EQ(v.gety(), -2.5);
}

TEST(test_vector, setters_chain) {
  common::vec2d v;
  const common::vec2d & ref = v.setx(3.0).sety(4.0);
  EXPECT_EQ(&ref, &v);
  EXPECT_EQ(v.getx(), 3.0);
  EXPECT_EQ(v.gety(), 4.0);
}

TEST(test_vector, norm_3_4_5) {
  EXPECT_DOUBLE_EQ((common::vec2d{3.0, 4.0}.norm2d()), 5.0);
}

TEST(test_vector, norm_zero) {
  EXPECT_EQ(common::vec2d{}.norm2d(), 0.0);
}

TEST(test_vector, norm_negative_components) {
  EXPECT_DOUBLE_EQ((common::vec2d{-3.0, -4.0}.norm2d()), 5.0);
}

TEST(test_vector, point_minus_point) {
  common::vec2d const v = common::point{5.0, 1.0} - common::point{2.0, 4.0};
  EXPECT_EQ(v.getx(), 3.0);
  EXPECT_EQ(v.gety(), -3.0);
}

TEST(test_vector, point_plus_vector) {
  common::point const p = common::point{1.0, 2.0} + common::vec2d{10.0, -5.0};
  EXPECT_EQ(p.getx(), 11.0);
  EXPECT_EQ(p.gety(), -3.0);
}

TEST(test_vector, vector_plus_vector) {
  common::vec2d const v = common::vec2d{1.0, 2.0} + common::vec2d{-4.0, 8.0};
  EXPECT_EQ(v.getx(), -3.0);
  EXPECT_EQ(v.gety(), 10.0);
}

TEST(test_vector, scalar_multiplication_commutes) {
  common::vec2d const v{2.0, -3.0};
  common::vec2d const right = v * 2.5;
  common::vec2d const left  = 2.5 * v;
  EXPECT_EQ(right.getx(), 5.0);
  EXPECT_EQ(right.gety(), -7.5);
  EXPECT_EQ(left.getx(), 5.0);
  EXPECT_EQ(left.gety(), -7.5);
}

TEST(test_vector, multiply_by_zero) {
  common::vec2d const v = common::vec2d{2.0, -3.0} * 0.0;
  EXPECT_EQ(v.getx(), 0.0);
  EXPECT_EQ(v.gety(), 0.0);
}

TEST(test_vector, constexpr_arithmetic) {
  constexpr common::vec2d v = common::vec2d{1.0, 2.0} + common::vec2d{3.0, 4.0};
  static_assert(v.getx() == 4.0 and v.gety() == 6.0);
  SUCCEED();
}
