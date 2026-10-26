#ifndef COMMON_COLLISION_RULES_HPP
#define COMMON_COLLISION_RULES_HPP

#include <cmath>

#include "particle_record.hpp"
#include "point.hpp"

namespace common {

  // Two particles collide when the distance between centres is strictly smaller than the sum of
  // their radii (touching is not a collision).
  [[nodiscard]] inline bool checkcollision(point const & p1, double r1, point const & p2,
                                           double r2) {
    double const dx = p1.x - p2.x;
    double const dy = p1.y - p2.y;
    return (r1 + r2) > std::sqrt(dx * dx + dy * dy);
  }

  // The absorber keeps its position and radius; mass and velocity are added.
  inline void merge_particles(particle_record & absorber, particle_record const & absorbed) {
    absorber.mass += absorbed.mass;
    absorber.vel = absorber.vel + absorbed.vel;
  }

}  // namespace common

#endif
