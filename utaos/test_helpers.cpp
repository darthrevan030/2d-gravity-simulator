#ifndef UTAOS_TEST_HELPERS_CPP
#define UTAOS_TEST_HELPERS_CPP

#include <cstddef>

#include "particle.hpp"

namespace test {

  [[nodiscard]] inline aos::particle make_particle(double mass, double radius, common::point pos,
                                                   common::vec2d vel = {}, std::size_t id = 0) {
    return aos::particle{id, mass, radius, pos, vel};  // record layout: id, mass, radius, pos, vel
  }

}  // namespace test

#endif
