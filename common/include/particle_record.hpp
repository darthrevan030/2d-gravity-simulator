// particle record

#ifndef COMMON_PARTICLE_RECORD_HPP
#define COMMON_PARTICLE_RECORD_HPP

#include "point.hpp"
#include "vector.hpp"

namespace common {

  struct particle_record {
    double mass{0.0};
    double radius{0.0};
    point pos{};
    vec2d vel{};
  };

}  // namespace common

#endif
