#include "point.hpp"

#include <gtest/gtest.h>

#include <sstream>

TEST(test_point, default_constructor) {
  common::point const p;
  EXPECT_EQ(p.x, 0.0);
  EXPECT_EQ(p.y, 0.0);
}

TEST(test_point, getters) {
  common::point const p{1.25, -7.5};
  EXPECT_EQ(p.getx(), 1.25);
  EXPECT_EQ(p.gety(), -7.5);
}

TEST(test_point, setters_chain) {
  common::point p;
  common::point & ref = p.setx(3.0).sety(4.0);
  EXPECT_EQ(&ref, &p);
  EXPECT_EQ(p.getx(), 3.0);
  EXPECT_EQ(p.gety(), 4.0);
}

TEST(test_point, setters_are_independent) {
  common::point p{1.0, 2.0};
  p.setx(9.0);
  EXPECT_EQ(p.getx(), 9.0);
  EXPECT_EQ(p.gety(), 2.0);
}

TEST(test_point, constexpr_usable) {
  constexpr common::point p = [] {
    common::point q;
    q.setx(1.0).sety(2.0);
    return q;
  }();
  static_assert(p.getx() == 1.0 && p.gety() == 2.0);
  SUCCEED();
}

TEST(test_point, extraction_operator) {
  std::istringstream iss("3.0 4.0");
  common::point p;
  iss >> p;
  EXPECT_EQ(p.x, 3.0);
  EXPECT_EQ(p.y, 4.0);
}

TEST(test_point, formatting) {
  common::point p{3.5, 4.5};
  std::string const expected = "3.5 4.5";
  std::string const actual = std::format("{}", p);
  EXPECT_EQ(actual, expected);
}

