// particle record

#ifndef COMMON_PARTICLE_RECORD_HPP
#define COMMON_PARTICLE_RECORD_HPP

#include "point.hpp"
#include "vector.hpp"

namespace common {

struct particle_record{
std::size_t id{0};
double mass{0.0};
double radius{0.0};
point pos;
vec2d vel;


};
}
#endif
