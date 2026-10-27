#include <gtest/gtest.h>

#include <vector>

#include "particle_system.hpp"
#include "test_helpers.hpp"

using test::make_particle;

namespace {
  // Collisions are resolved when the system is built, so the constructor is enough to test them.
  aos::particle_system build(std::vector<aos::particle> particles) {
    return aos::particle_system{std::move(particles)};
  }
}  // namespace

TEST(test_collisions, heavier_first_particle_absorbs) {
  auto const system = build({make_particle(10.0, 2.0, {50.0, 50.0}), make_particle(5.0, 2.0, {51.0, 50.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 15.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 50.0);
}

TEST(test_collisions, heavier_second_particle_absorbs) {
  auto const system = build({make_particle(5.0, 2.0, {50.0, 50.0}), make_particle(10.0, 1.0, {51.0, 50.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 15.0);
  EXPECT_DOUBLE_EQ(ps.at(0).radius, 1.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 51.0);
}

TEST(test_collisions, equal_mass_larger_radius_absorbs) {
  auto const system = build({make_particle(10.0, 1.0, {50.0, 50.0}), make_particle(10.0, 2.0, {51.0, 50.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).radius, 2.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 51.0);
}

TEST(test_collisions, equal_mass_and_radius_lower_index_absorbs) {
  auto const system = build({make_particle(10.0, 1.0, {50.0, 50.0}), make_particle(10.0, 1.0, {51.0, 50.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 20.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 50.0);
}

TEST(test_collisions, merge_adds_velocities) {
  auto const system = build({make_particle(10.0, 1.0, {50.0, 50.0}, {1.0, 0.0}),
                             make_particle(10.0, 1.0, {51.0, 50.0}, {0.0, 1.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 1.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.y, 1.0);
}

// distance == r1 + r2 is touching, not colliding (ftest/cases/touching_no_collision)
TEST(test_collisions, touching_particles_do_not_merge) {
  auto const system = build({make_particle(1.0, 2.0, {10.0, 10.0}), make_particle(1.0, 2.0, {14.0, 10.0})});
  EXPECT_EQ(system.particles().size(), 2U);
}

TEST(test_collisions, slightly_overlapping_particles_merge) {
  auto const system = build({make_particle(1.0, 2.0, {10.0, 10.0}), make_particle(1.0, 2.0, {13.999, 10.0})});
  EXPECT_EQ(system.particles().size(), 1U);
}

TEST(test_collisions, distance_uses_both_axes) {
  // dx = 3, dy = 4 -> distance 5; radii sum 6 collides, radii sum 5 only touches
  EXPECT_EQ(build({make_particle(1.0, 3.0, {50.0, 50.0}), make_particle(1.0, 3.0, {53.0, 54.0})}).particles().size(), 1U);
  EXPECT_EQ(build({make_particle(1.0, 2.5, {50.0, 50.0}), make_particle(1.0, 2.5, {53.0, 54.0})}).particles().size(), 2U);
}

TEST(test_collisions, distant_particles_do_not_merge) {
  auto const system = build({make_particle(1.0, 1.0, {10.0, 10.0}), make_particle(1.0, 1.0, {90.0, 90.0})});
  EXPECT_EQ(system.particles().size(), 2U);
}

// ftest/cases/merge_chain: 0 absorbs into 1, the result then absorbs 2, particle 3 stays alone
TEST(test_collisions, chain_of_merges) {
  auto const system = build({make_particle(5.0, 1.0, {50.0, 50.0}, {1.0, 0.0}),
                             make_particle(10.0, 1.5, {52.0, 50.0}, {0.0, 1.0}),
                             make_particle(3.0, 1.0, {54.0, 50.0}, {2.0, 2.0}),
                             make_particle(7.0, 1.0, {80.0, 80.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 2U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 18.0);
  EXPECT_DOUBLE_EQ(ps.at(0).radius, 1.5);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 52.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 3.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.y, 3.0);
  EXPECT_DOUBLE_EQ(ps.at(1).mass, 7.0);
}

// ftest/cases/collapse_to_one
TEST(test_collisions, everything_collapses_to_one) {
  auto const system = build({make_particle(10.0, 5.0, {20.0, 20.0}, {0.0, 0.0}),
                             make_particle(11.0, 5.0, {24.0, 20.0}, {1.0, 0.0}),
                             make_particle(12.0, 5.0, {28.0, 20.0}, {0.0, 1.0}),
                             make_particle(13.0, 5.0, {24.0, 24.0}, {-1.0, 0.0}),
                             make_particle(14.0, 5.0, {20.0, 28.0}, {0.0, -1.0})});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 60.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 24.0);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.y, 20.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.x, 0.0);
  EXPECT_DOUBLE_EQ(ps.at(0).vel.y, 0.0);
}

TEST(test_collisions, particles_merge_after_moving_into_each_other) {
  aos::particle_system system{
      {make_particle(1.0, 1.0, {50.0, 50.0}, {1.0, 0.0}), make_particle(1.0, 1.0, {53.0, 50.0}, {-1.0, 0.0})}};
  ASSERT_EQ(system.particles().size(), 2U);
  system.simulate(1, 1.0, 100.0, 100.0);  // they end up 1 apart, radii sum is 2
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 2.0);
  EXPECT_NEAR(ps.at(0).pos.x, 51.0, 1e-6);
}

TEST(test_collisions, empty_and_single_particle_have_no_collisions) {
  EXPECT_TRUE(build({}).particles().empty());
  EXPECT_EQ(build({make_particle(1.0, 1.0, {50.0, 50.0})}).particles().size(), 1U);
}

TEST(test_collisions, equal_mass_and_radius_smaller_id_absorbs) {
  auto const system = build({make_particle(10.0, 1.0, {50.0, 50.0}, {}, 7),
                             make_particle(10.0, 1.0, {51.0, 50.0}, {}, 3)});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_EQ(ps.at(0).id, 3U);
  EXPECT_DOUBLE_EQ(ps.at(0).pos.x, 51.0);
}

TEST(test_collisions, absorber_keeps_its_id) {
  auto const system = build({make_particle(5.0, 2.0, {50.0, 50.0}, {}, 0),
                             make_particle(10.0, 1.0, {51.0, 50.0}, {}, 1)});
  auto const & ps = system.particles();
  ASSERT_EQ(ps.size(), 1U);
  EXPECT_EQ(ps.at(0).id, 1U);
  EXPECT_DOUBLE_EQ(ps.at(0).mass, 15.0);
}
