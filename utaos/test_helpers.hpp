#ifndef UTAOS_TEST_HELPERS_HPP
#define UTAOS_TEST_HELPERS_HPP

#include <cstddef>

#include "particle.hpp"

namespace test {

  [[nodiscard]] inline aos::particle make_particle(double mass, double radius, common::point pos,
                                                   common::vec2d vel = {}) {
    return aos::particle{0, mass, radius, pos, vel};  // record layout: id, mass, radius, pos, vel
  }

  [[nodiscard]] inline aos::particle with_id(aos::particle p, std::size_t id) {
    p.id = id;
    return p;
  }

}  // namespace test

#endif
