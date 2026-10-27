#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <vector>

#include "particle_system.hpp"
#include "reference_sim.hpp"
#include "scenarios.hpp"

namespace {

  // Relative tolerance: both sides do the same maths in a different order, so results agree to
  // rounding error, not bit for bit.
  void expect_close(double actual, double expected) {
    double const tol = 1e-9 * std::max({1.0, std::abs(actual), std::abs(expected)});
    EXPECT_NEAR(actual, expected, tol);
  }

  [[nodiscard]] std::vector<aos::particle> to_aos(std::vector<reference::body> const & bodies) {
    std::vector<aos::particle> out;
    out.reserve(bodies.size());
    for (auto const & b : bodies) {
      out.push_back(aos::particle{b.mass, b.radius, common::point{b.x, b.y}, common::vec2d{b.vx, b.vy}});
    }
    return out;
  }

  // Runs both implementations on the same scenario and compares every particle.
  // Returns the number of survivors so callers can check collisions really happened.
  std::size_t run_and_compare(testutil::scenario const & s) {
    auto const expected = reference::simulate(s.bodies, {s.iterations, s.dt, s.width, s.height});

    aos::particle_system system{to_aos(s.bodies)};
    system.simulate(s.iterations, s.dt, s.width, s.height);
    auto const & actual = system.particles();

    EXPECT_EQ(actual.size(), expected.size());
    if (actual.size() != expected.size()) { return actual.size(); }

    for (std::size_t i = 0; i < expected.size(); ++i) {
      SCOPED_TRACE("particle " + std::to_string(i));
      expect_close(actual.at(i).mass, expected.at(i).mass);
      expect_close(actual.at(i).radius, expected.at(i).radius);
      expect_close(actual.at(i).pos.x, expected.at(i).x);
      expect_close(actual.at(i).pos.y, expected.at(i).y);
      expect_close(actual.at(i).vel.x, expected.at(i).vx);
      expect_close(actual.at(i).vel.y, expected.at(i).vy);
    }
    return actual.size();
  }

}  // namespace

TEST(test_equivalence, single_particle) {
  for (unsigned seed = 1; seed <= 10; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    run_and_compare(testutil::make_scenario(seed, 1, 1000.0, 100));
  }
}

TEST(test_equivalence, two_particles) {
  for (unsigned seed = 1; seed <= 20; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    run_and_compare(testutil::make_scenario(seed, 2, 500.0, 100));
  }
}

TEST(test_equivalence, sparse_world_few_collisions) {
  for (unsigned seed = 1; seed <= 20; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    run_and_compare(testutil::make_scenario(seed, 20, 5000.0, 100));
  }
}

TEST(test_equivalence, dense_world_many_collisions) {
  std::size_t total_merged = 0;
  for (unsigned seed = 1; seed <= 20; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    auto const s = testutil::make_scenario(seed, 30, 100.0, 50);
    total_merged += s.bodies.size() - run_and_compare(s);
  }
  // the scenario is only a useful test if merges actually happen
  EXPECT_GT(total_merged, 0U);
}

TEST(test_equivalence, many_particles_long_run) {
  for (unsigned seed = 1; seed <= 3; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    run_and_compare(testutil::make_scenario(seed, 100, 2000.0, 200));
  }
}

TEST(test_equivalence, overlapping_at_start) {
  // tiny world: most particles overlap before the first step
  for (unsigned seed = 1; seed <= 10; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    auto const s = testutil::make_scenario(seed, 15, 30.0, 20);
    EXPECT_LT(run_and_compare(s), s.bodies.size());
  }
}

TEST(test_equivalence, zero_iterations_only_merges_initial_overlaps) {
  for (unsigned seed = 1; seed <= 10; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    run_and_compare(testutil::make_scenario(seed, 15, 30.0, 0));
  }
}

TEST(test_equivalence, small_and_large_time_steps) {
  for (double const dt : {0.001, 0.1, 1.0}) {
    SCOPED_TRACE("dt " + std::to_string(dt));
    auto s = testutil::make_scenario(7, 10, 500.0, 50);
    s.dt = dt;
    run_and_compare(s);
  }
}
