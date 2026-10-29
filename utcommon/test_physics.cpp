#include <gtest/gtest.h>

#include "constants.hpp"
#include "physics.hpp"
#include "point.hpp"
#include "vector.hpp"
#include "enclosure.hpp"
namespace {
constexpr double width  = 100.0;
constexpr double height = 100.0;
constexpr double radius = 1.0;
constexpr common::enclosure box{width, height};
} 

TEST(Physics, ForceKnownValue) {
  common::point const p1{0.0, 0.0};
  common::point const p2{3.0, 4.0};

  common::vec2d const force = common::compute_force(p1, 1.0, p2, 1.0);

  double const expected_x = common::constants::G * 0.024;
  double const expected_y = common::constants::G * 0.032;
  EXPECT_NEAR(force.getx(), expected_x, 1e-12 * expected_x);
  EXPECT_NEAR(force.gety(), expected_y, 1e-12 * expected_y);
}
TEST(Physics, ForceIsOppositeWhenSwapped) {
  common::point const p1{1.0, 2.0};
  common::point const p2{7.0, -3.0};

  common::vec2d const f12 = common::compute_force(p1, 2.0, p2, 5.0);
  common::vec2d const f21 = common::compute_force(p2, 5.0, p1, 2.0);

  EXPECT_DOUBLE_EQ(f12.getx(), -f21.getx());
  EXPECT_DOUBLE_EQ(f12.gety(), -f21.gety());
}
TEST(Physics, KinematicUpdate){
  common::point pos{0.0,0.0};
  common::vec2d vel{1.0,2.0};
  common::vec2d const acc{0.5,-1.0};

  common:: update_kinematics(pos, vel, acc, 1.0);
  EXPECT_NEAR(pos.getx(), 1.5, 1e-9);
  EXPECT_NEAR(pos.gety(), 1.0, 1e-9);

  EXPECT_NEAR(vel.getx(), 1.5, 1e-9);
  EXPECT_NEAR(vel.gety(), 1.0, 1e-9);


}

TEST(Physics, Reboundupperwall){

  common::point pos{50.0, 99.5};
  common::vec2d vel{4.0, 5.0};
  common::apply_rebound(pos,vel,1.0,box);

  EXPECT_DOUBLE_EQ(pos.getx(), 50.0);   // Horizontal position unchanged
  EXPECT_DOUBLE_EQ(vel.getx(), 4.0);    // Horizontal velocity unchanged
  EXPECT_DOUBLE_EQ(pos.gety(), 99.0);   // Clamped to height - radius (100 - 1)
  EXPECT_DOUBLE_EQ(vel.gety(), -5.0);    // -|vy|


}
TEST(Physics, ReboundRightWall) {


  // Positioned past the right wall threshold (100.0 - 1.0 = 99.0)
  common::point pos{99.5, 50.0};
  common::vec2d vel{4.0, 5.0};

  common::apply_rebound(pos, vel, radius, box);

  EXPECT_DOUBLE_EQ(pos.getx(), 99.0);   // Clamped to width - radius (100 - 1)
  EXPECT_DOUBLE_EQ(vel.getx(), -4.0);   // Horizontal velocity reversed
  EXPECT_DOUBLE_EQ(pos.gety(), 50.0);   // Vertical position unchanged
  EXPECT_DOUBLE_EQ(vel.gety(), 5.0);    // Vertical velocity unchanged
}

TEST(Physics, ReboundLowerWall){
  common::point pos{50.0, 0.5};
  common::vec2d vel{3.0, -2.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.gety(), 1.0);
  EXPECT_DOUBLE_EQ(vel.gety(), 2.0);
  EXPECT_DOUBLE_EQ(pos.getx(), 50.0);
  EXPECT_DOUBLE_EQ(vel.getx(), 3.0);
}

TEST(Physics, ReboundLeftWall){
  common::point pos{0.5, 30.0};
  common::vec2d vel{-2.0, 3.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.getx(), 1.0);
  EXPECT_DOUBLE_EQ(vel.getx(), 2.0);
  EXPECT_DOUBLE_EQ(pos.gety(), 30.0);
  EXPECT_DOUBLE_EQ(vel.gety(), 3.0);
}
TEST(Physics, ReboundBottomLeftCorner) {
  common::point pos{0.5, 0.5};
  common::vec2d vel{-1.0, -1.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.getx(), 1.0);
  EXPECT_DOUBLE_EQ(pos.gety(), 1.0);
  EXPECT_DOUBLE_EQ(vel.getx(), 1.0);
  EXPECT_DOUBLE_EQ(vel.gety(), 1.0);
}

TEST(Physics, ForceIsZeroForCoincidentParticles) {
  common::point const p{5.0, 5.0};

  common::vec2d const force = common::compute_force(p, 1.0, p, 1.0);

  EXPECT_EQ(force.getx(), 0.0);  // guarded, not 0/0
  EXPECT_EQ(force.gety(), 0.0);
}

TEST(Physics, ForceJustAboveEpsilonIsNotZeroed) {
  common::point const p1{0.0, 0.0};
  common::point const p2{1e-6, 0.0};  // above EPSILON (1e-9), so the guard must not apply

  common::vec2d const force = common::compute_force(p1, 1.0, p2, 1.0);

  double const expected_x = common::constants::G * 1e12;  // G * m1 * m2 / d^2
  EXPECT_NEAR(force.getx(), expected_x, 1e-9 * expected_x);
  EXPECT_DOUBLE_EQ(force.gety(), 0.0);
}

// Past a wall but already moving back into the box: clamp the position, keep the velocity.
TEST(Physics, ReboundUpperWallMovingAway) {
  common::point pos{50.0, 99.5};
  common::vec2d vel{4.0, -5.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.gety(), 99.0);
  EXPECT_DOUBLE_EQ(vel.gety(), -5.0);
}

TEST(Physics, ReboundRightWallMovingAway) {
  common::point pos{99.5, 30.0};
  common::vec2d vel{-3.0, 2.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.getx(), 99.0);
  EXPECT_DOUBLE_EQ(vel.getx(), -3.0);
}

TEST(Physics, ReboundLowerWallMovingAway) {
  common::point pos{30.0, 0.5};
  common::vec2d vel{2.0, 3.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.gety(), 1.0);
  EXPECT_DOUBLE_EQ(vel.gety(), 3.0);
}

TEST(Physics, ReboundLeftWallMovingAway) {
  common::point pos{0.5, 30.0};
  common::vec2d vel{2.0, 3.0};

  common::apply_rebound(pos, vel, 1.0, box);

  EXPECT_DOUBLE_EQ(pos.getx(), 1.0);
  EXPECT_DOUBLE_EQ(vel.getx(), 2.0);
}
