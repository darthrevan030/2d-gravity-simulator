#include <cstddef>
#include <vector>

#include "particle_system.hpp"
#include "physics.hpp"

namespace aos {

  // Sums the forces first and divides by the mass once per particle at the end, instead of two
  // divisions per pair. Particle i's sum is kept in locals so it isn't reloaded every pair.
  std::vector<common::vec2d> particle_system::compute_accelerations() const {
    std::vector<common::vec2d> acc(particles_.size(), common::vec2d{});
    for (std::size_t i = 0; i < particles_.size(); ++i) {
      particle const & pi = particles_.at(i);
      double fxi = 0.0;
      double fyi = 0.0;
      for (std::size_t j = i + 1; j < particles_.size(); ++j) {
        particle const & pj = particles_.at(j);
        // force on i points towards j; j feels the opposite
        auto const force = common::compute_force(pi.pos, pi.mass, pj.pos, pj.mass);
        fxi += force.x;
        fyi += force.y;
        acc.at(j).x -= force.x;
        acc.at(j).y -= force.y;
      }
      acc.at(i).x += fxi;
      acc.at(i).y += fyi;
    }
    for (std::size_t i = 0; i < particles_.size(); ++i) {  // a = F / m
      acc.at(i).x /= particles_.at(i).mass;
      acc.at(i).y /= particles_.at(i).mass;
    }
    return acc;
  }

}  // namespace aos
