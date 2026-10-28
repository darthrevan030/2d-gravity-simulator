#include <gtest/gtest.h>

#include <vector>

#include "reference_sim.hpp"

// The reference is the oracle for the equivalence tests, so it is checked against numbers worked
// out independently (ftest/cases) before anything is compared to it.

TEST(test_reference, lone_particle_moves_in_straight_line) {
  auto const out = reference::simulate({{1.0, 1.0, 10.0, 10.0, 1.0, 2.0}}, {10, 0.1, 100.0, 100.0});
  ASSERT_EQ(out.size(), 1U);
  EXPECT_NEAR(out.at(0).x, 11.0, 1e-9);
  EXPECT_NEAR(out.at(0).y, 12.0, 1e-9);
}

TEST(test_reference, two_body_gravity_matches_ftest_case) {
  auto const out = reference::simulate(
      {{1e12, 1.0, 100.0, 100.0, 0.0, 0.0}, {1e12, 1.0, 200.0, 100.0, 0.0, 0.0}}, {2, 1.0, 300.0, 200.0});
  ASSERT_EQ(out.size(), 2U);
  EXPECT_NEAR(out.at(0).x, 100.020, 1e-3);
  EXPECT_NEAR(out.at(0).vx, 0.013, 1e-3);
  EXPECT_NEAR(out.at(1).x, 199.980, 1e-3);
  EXPECT_NEAR(out.at(1).vx, -0.013, 1e-3);
}

TEST(test_reference, merge_chain_matches_ftest_case) {
  auto const out = reference::simulate({{5.0, 1.0, 50.0, 50.0, 1.0, 0.0},
                                        {10.0, 1.5, 52.0, 50.0, 0.0, 1.0},
                                        {3.0, 1.0, 54.0, 50.0, 2.0, 2.0},
                                        {7.0, 1.0, 80.0, 80.0, 0.0, 0.0},},
                                       {1, 0.1, 100.0, 100.0});
  ASSERT_EQ(out.size(), 2U);
  EXPECT_NEAR(out.at(0).mass, 18.0, 1e-9);
  EXPECT_NEAR(out.at(0).x, 52.3, 1e-3);
  EXPECT_NEAR(out.at(0).vx, 3.0, 1e-3);
  EXPECT_NEAR(out.at(1).mass, 7.0, 1e-9);
}

TEST(test_reference, touching_particles_do_not_merge) {
  auto const out = reference::simulate(
      {{1.0, 2.0, 10.0, 10.0, -1.0, 0.0}, {1.0, 2.0, 14.0, 10.0, 1.0, 0.0}}, {1, 0.1, 100.0, 100.0});
  EXPECT_EQ(out.size(), 2U);
}

TEST(test_reference, rebounds_off_left_wall) {
  auto const out = reference::simulate({{1.0, 1.0, 2.0, 50.0, -5.0, 0.0}}, {1, 1.0, 100.0, 100.0});
  ASSERT_EQ(out.size(), 1U);
  EXPECT_DOUBLE_EQ(out.at(0).x, 1.0);
  EXPECT_DOUBLE_EQ(out.at(0).vx, 5.0);
}

TEST(test_reference, collapse_to_one_matches_ftest_case) {
  auto const out = reference::simulate({{10.0, 5.0, 20.0, 20.0, 0.0, 0.0},
                                        {11.0, 5.0, 24.0, 20.0, 1.0, 0.0},
                                        {12.0, 5.0, 28.0, 20.0, 0.0, 1.0},
                                        {13.0, 5.0, 24.0, 24.0, -1.0, 0.0},
                                        {14.0, 5.0, 20.0, 28.0, 0.0, -1.0},},
                                       {3, 0.5, 50.0, 50.0});
  ASSERT_EQ(out.size(), 1U);
  EXPECT_NEAR(out.at(0).mass, 60.0, 1e-9);
  EXPECT_NEAR(out.at(0).x, 24.0, 1e-3);
  EXPECT_NEAR(out.at(0).y, 20.0, 1e-3);
}
