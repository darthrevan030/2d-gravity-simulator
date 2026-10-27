#ifndef UTAOS_TEST_HELPERS_HPP
#define UTAOS_TEST_HELPERS_HPP

#include <cstddef>

#include "particle.hpp"

namespace test {

  [[nodiscard]] inline aos::particle make_particle(double mass, double radius, common::point pos,
                                                   common::vec2d vel = {}, std::size_t id = 0) {
    return aos::particle{.id = id, .mass = mass, .radius = radius, .pos = pos, .vel = vel};
  }

}  // namespace test

#endif
