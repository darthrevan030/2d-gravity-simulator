#include <gtest/gtest.h>

#include <cmath>

#include "particle_system.hpp"
#include "test_helpers.cpp"

using test::make_particle;

TEST(test_integrate, empty_system_does_not_throw) {
  aos::particle_system system{{}};
  EXPECT_NO_THROW(system.simulate(5, 0.1, 100.0, 100.0));
  EXPECT_TRUE(system.particles().empty());
}

TEST(test_integrate, lone_particle_moves_in_straight_line) {
  aos::particle_system system{{make_particle(1.0, 1.0, {10.0, 10.0}, {1.0, 2.0})}};
  system.simulate(10, 0.1, 100.0, 100.0);
  auto const & p = system.particles().at(0);
  EXPECT_NEAR(p.pos.x, 11.0, 1e-9);
  EXPECT_NEAR(p.pos.y, 12.0, 1e-9);
  EXPECT_DOUBLE_EQ(p.vel.x, 1.0);
  EXPECT_DOUBLE_EQ(p.vel.y, 2.0);
}

// ftest/cases/two_body_gravity: m = 1e12 each, 100 apart, dt = 1, 2 iterations
TEST(test_integrate, two_body_gravity_matches_reference) {
  aos::particle_system system{
      {make_particle(1e12, 1.0, {100.0, 100.0}), make_particle(1e12, 1.0, {200.0, 100.0})}};
  system.simulate(2, 1.0, 300.0, 200.0);
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 2U);
  EXPECT_NEAR(ps.at(0).pos.x, 100.020, 1e-3);
  EXPECT_NEAR(ps.at(0).vel.x, 0.013, 1e-3);
  EXPECT_NEAR(ps.at(1).pos.x, 199.980, 1e-3);
  EXPECT_NEAR(ps.at(1).vel.x, -0.013, 1e-3);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.y, 100.0);
  EXPECT_DOUBLE_EQ(ps.at(1).pos.y, 100.0);
}

TEST(test_integrate, particles_attract_each_other) {
  aos::particle_system system{
      {make_particle(1e12, 1.0, {100.0, 100.0}), make_particle(1e12, 1.0, {100.0, 180.0})}};
  system.simulate(1, 1.0, 300.0, 300.0);
  auto const & ps = system.particles();
  EXPECT_GT(ps.at(0).vel.y, 0.0);  // pulled towards the one above
  EXPECT_LT(ps.at(1).vel.y, 0.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 0.0);
}

// equal and opposite forces: total momentum stays zero even with unequal masses
TEST(test_integrate, momentum_is_conserved_for_unequal_masses) {
  aos::particle_system system{
      {make_particle(1e12, 1.0, {100.0, 100.0}), make_particle(4e12, 1.0, {200.0, 150.0})}};
  system.simulate(3, 1.0, 400.0, 400.0);
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 2U);
  double const px = ps.at(0).mass * ps.at(0).vel.x + ps.at(1).mass * ps.at(1).vel.x;
  double const py = ps.at(0).mass * ps.at(0).vel.y + ps.at(1).mass * ps.at(1).vel.y;
  double const scale = ps.at(0).mass * (std::abs(ps.at(0).vel.x) + std::abs(ps.at(0).vel.y));
  ASSERT_GT(scale, 0.0);
  EXPECT_NEAR(px / scale, 0.0, 1e-12);
  EXPECT_NEAR(py / scale, 0.0, 1e-12);
}

TEST(test_integrate, far_light_particles_barely_interact) {
  aos::particle_system system{{make_particle(1.0, 1.0, {10.0, 10.0}), make_particle(1.0, 1.0, {90.0, 90.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  EXPECT_NEAR(system.particles().at(0).vel.x, 0.0, 1e-9);
}

// --- walls: position is clamped to [r, limit - r] and the velocity component flips ---

TEST(test_integrate, rebounds_off_left_wall) {
  aos::particle_system system{{make_particle(1.0, 1.0, {2.0, 50.0}, {-5.0, 0.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);  // would reach x = -3
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.x, 1.0);
  EXPECT_DOUBLE_EQ(p.vel.x, 5.0);
}

TEST(test_integrate, rebounds_off_right_wall) {
  aos::particle_system system{{make_particle(1.0, 1.0, {98.0, 50.0}, {5.0, 0.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.x, 99.0);
  EXPECT_DOUBLE_EQ(p.vel.x, -5.0);
}

TEST(test_integrate, rebounds_off_bottom_wall) {
  aos::particle_system system{{make_particle(1.0, 1.0, {50.0, 2.0}, {0.0, -5.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.y, 1.0);
  EXPECT_DOUBLE_EQ(p.vel.y, 5.0);
}

TEST(test_integrate, rebounds_off_top_wall) {
  aos::particle_system system{{make_particle(1.0, 1.0, {50.0, 98.0}, {0.0, 5.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.y, 99.0);
  EXPECT_DOUBLE_EQ(p.vel.y, -5.0);
}

TEST(test_integrate, rebounds_off_corner_on_both_axes) {
  aos::particle_system system{{make_particle(1.0, 1.0, {2.0, 2.0}, {-5.0, -3.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.x, 1.0);
  EXPECT_DOUBLE_EQ(p.pos.y, 1.0);
  EXPECT_DOUBLE_EQ(p.vel.x, 5.0);
  EXPECT_DOUBLE_EQ(p.vel.y, 3.0);
}

TEST(test_integrate, velocity_moving_away_from_wall_is_not_flipped) {
  aos::particle_system system{{make_particle(1.0, 1.0, {0.2, 50.0}, {1.0, 0.0})}};
  system.simulate(1, 0.0, 100.0, 100.0);  // dt = 0: only the wall clamp acts
  auto const & p = system.particles().at(0);
  EXPECT_DOUBLE_EQ(p.pos.x, 1.0);
  EXPECT_DOUBLE_EQ(p.vel.x, 1.0);
}

TEST(test_integrate, wall_radius_is_respected) {
  aos::particle_system system{{make_particle(1.0, 5.0, {8.0, 50.0}, {-10.0, 0.0})}};
  system.simulate(1, 1.0, 100.0, 100.0);
  EXPECT_DOUBLE_EQ(system.particles().at(0).pos.x, 5.0);
}
