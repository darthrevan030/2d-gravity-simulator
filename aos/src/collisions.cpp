#include <cstddef>
#include <vector>

#include "collision_rules.hpp"
#include "particle_system.hpp"

namespace aos {

  // Merges overlapping pairs in lexicographic order. A merge only changes the survivor's mass and
  // velocity, never its position or radius, so overlaps cannot appear, only disappear. One forward
  // pass therefore equals "restart after every merge". Dead particles are removed once at the end.
  void particle_system::process_collisions() {
    std::size_t const n = particles_.size();
    std::vector<char> alive(n, 1);

    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = i + 1; j < n and alive.at(i) != 0; ++j) {
        if (alive.at(j) == 0) { continue; }
        if (!common::checkcollision(particles_.at(i).pos, particles_.at(i).radius, particles_.at(j).pos,
                                    particles_.at(j).radius)) {
          continue;
        }

        std::size_t absorber = i;
        std::size_t absorbed = j;
        if (particles_.at(j).mass > particles_.at(i).mass or
            (particles_.at(j).mass == particles_.at(i).mass and
             (particles_.at(j).radius > particles_.at(i).radius or
              (particles_.at(j).radius == particles_.at(i).radius and
               particles_.at(j).id < particles_.at(i).id)))) {
          absorber = j;
          absorbed = i;
        }

        common::merge_particles(particles_.at(absorber), particles_.at(absorbed));
        alive.at(absorbed) = 0;
      }
    }

    std::size_t kept = 0;
    for (std::size_t i = 0; i < n; ++i) {
      if (alive.at(i) != 0) {
        if (kept != i) { particles_.at(kept) = particles_.at(i); }
        ++kept;
      }
    }
    particles_.erase(particles_.begin() + kept, particles_.end());
  }

}  // namespace aos
