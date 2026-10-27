#include <gtest/gtest.h>

#include <vector>

#include "particle_system.hpp"
#include "test_helpers.hpp"

using test::make_particle;

TEST(test_particle_system, empty_system) {
  aos::particle_system const system{std::vector<aos::particle>{}};
  EXPECT_TRUE(system.particles().empty());
}

TEST(test_particle_system, keeps_separate_particles_in_order) {
  aos::particle_system const system{{make_particle(1.0, 1.0, {10.0, 10.0}),
                                      make_particle(2.0, 1.0, {50.0, 50.0}),
                                      make_particle(3.0, 1.0, {90.0, 90.0})}};
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 3U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 1.0);
  EXPECT_DOUBLE_EQ(ps.at(1).mass, 2.0);
  EXPECT_DOUBLE_EQ(ps.at(2).mass, 3.0);
}

// ftest/cases/init_merge_by_mass: overlapping particles are merged on construction
TEST(test_particle_system, constructor_merges_overlapping_particles) {
  aos::particle_system const system{
      {make_particle(10.0, 2.0, {50.0, 50.0}, {1.0, 0.0}), make_particle(20.0, 1.0, {51.0, 50.0}, {0.0, 1.0})}};
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 30.0);
  EXPECT_DOUBLE_EQ(ps.at(0).radius, 1.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 51.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.y, 50.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 1.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.y, 1.0);
}

TEST(test_particle_system, zero_iterations_changes_nothing) {
  aos::particle_system system{{make_particle(1.0, 1.0, {20.0, 30.0}, {4.0, 5.0})}};
  system.simulate(0, 0.5, 100.0, 100.0);
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 20.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.y, 30.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 4.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.y, 5.0);
}

TEST(test_particle_system, particles_returns_stable_view) {
  aos::particle_system system{{make_particle(1.0, 1.0, {20.0, 30.0})}};
  auto const * before = &system.particles();
  system.simulate(3, 0.1, 100.0, 100.0);
  EXPECT_EQ(before, &system.particles());
}
