#ifndef UTAOS_TEST_HELPERS_HPP
#define UTAOS_TEST_HELPERS_HPP

#include "particle.hpp"

namespace test {

  [[nodiscard]] inline aos::particle make_particle(double mass, double radius, common::point pos,
                                                   common::vec2d vel = {}) {
    return aos::particle{mass, radius, pos, vel};
  }

}  // namespace test

#endif
