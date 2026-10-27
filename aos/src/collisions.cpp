#include <cstddef>

#include "collision_rules.hpp"
#include "particle_system.hpp"

namespace aos {

  // Merges overlapping pairs until none remain. Pairs are scanned in index order and the scan
  // restarts after every merge. The heavier particle absorbs the lighter one; ties go to the
  // larger radius, then to the lower index.
  void particle_system::process_collisions() {
    bool collision_found = true;
    while (collision_found) {
      collision_found = false;

      for (std::size_t i = 0; i < particles_.size() && !collision_found; ++i) {
        for (std::size_t j = i + 1; j < particles_.size(); ++j) {
          if (!common::checkcollision(particles_.at(i).pos, particles_.at(i).radius, particles_.at(j).pos,
                                      particles_.at(j).radius)) {
            continue;
          }

          std::size_t absorber = i;
          std::size_t absorbed = j;

          if (particles_.at(j).mass > particles_.at(i).mass ||
              (particles_.at(j).mass == particles_.at(i).mass &&
               particles_.at(j).radius > particles_.at(i).radius)) {
            absorber = j;
            absorbed = i;
          }

          common::merge_particles(particles_[absorber], particles_[absorbed]);
          particles_.erase(particles_.begin() + static_cast<std::ptrdiff_t>(absorbed));
          collision_found = true;
          break;
        }
      }
    }
  }

}  // namespace aos
