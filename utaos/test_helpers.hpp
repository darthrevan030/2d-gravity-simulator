#ifndef UTAOS_TEST_HELPERS_HPP
#define UTAOS_TEST_HELPERS_HPP

#include "particle.hpp"

namespace test {

  [[nodiscard]] inline aos::particle make_particle(double mass, double radius, double x, double y, double vx = 0.0, double vy = 0.0) {
    return aos::particle{mass, radius, common::point{x, y}, common::vec2d{vx, vy}};
  }

}  // namespace test

#endif
