#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <vector>

#include "collision_grid.hpp"
#include "point.hpp"

namespace {

  struct circle {
    common::point centre;
    double radius;
  };

  [[nodiscard]] std::vector<common::point> centres_of(std::vector<circle> const & circles) {
    std::vector<common::point> out;
    out.reserve(circles.size());
    std::ranges::transform(circles, std::back_inserter(out), &circle::centre);
    return out;
  }

  [[nodiscard]] double max_radius_of(std::vector<circle> const & circles) {
    return std::ranges::max(circles, {}, &circle::radius).radius;
  }

  [[nodiscard]] bool overlap(circle const & a, circle const & b) {
    return std::hypot(a.centre.x - b.centre.x, a.centre.y - b.centre.y) < a.radius + b.radius;
  }

}  // namespace

TEST(CollisionGrid, FindsEveryOverlappingLaterParticle) {
  // 2000 circles spread over a 100 x 100 box by fractional parts of irrational steps: fixed, so the
  // test is repeatable, but irregular enough to give many overlaps in different cells
  std::vector<circle> circles;
  circles.reserve(2000);
  for (int k = 0; k < 2000; ++k) {
    auto const kd = static_cast<double>(k);
    circles.push_back({{std::fmod(kd * 61.803398875, 100.0), std::fmod(kd * 41.421356237, 100.0)},
                       0.1 + std::fmod(kd * 0.7320508076, 1.9)});
  }
  auto const centres = centres_of(circles);
  common::collision_grid const grid{centres, max_radius_of(circles)};

  std::vector<std::size_t> near;
  std::size_t overlaps = 0;
  for (std::size_t i = 0; i < circles.size(); ++i) {
    grid.later_neighbours(i, near);
    EXPECT_TRUE(std::ranges::is_sorted(near));
    EXPECT_TRUE(std::ranges::all_of(near, [i](std::size_t j) { return j > i; }));
    for (std::size_t j = i + 1; j < circles.size(); ++j) {
      if (not overlap(circles.at(i), circles.at(j))) { continue; }
      ++overlaps;
      EXPECT_TRUE(std::ranges::binary_search(near, j)) << "pair " << i << ", " << j << " missed";
    }
  }
  EXPECT_GT(overlaps, 0U);  // the test only means something if some pairs overlap
}

TEST(CollisionGrid, FarParticlesAreNotCandidates) {
  std::vector<circle> const circles{{{0.0, 0.0}, 1.0}, {{1.0, 0.0}, 1.0}, {{500.0, 500.0}, 1.0}};
  auto const centres = centres_of(circles);
  common::collision_grid const grid{centres, max_radius_of(circles)};
  std::vector<std::size_t> near;
  grid.later_neighbours(0, near);
  EXPECT_EQ(near, (std::vector<std::size_t>{1}));
  grid.later_neighbours(2, near);
  EXPECT_TRUE(near.empty());
}

TEST(CollisionGrid, SingleParticleHasNoCandidates) {
  std::vector<common::point> const centres{{10.0, 10.0}};
  common::collision_grid const grid{centres, 1.0};
  std::vector<std::size_t> near{7};
  grid.later_neighbours(0, near);
  EXPECT_TRUE(near.empty());
}
