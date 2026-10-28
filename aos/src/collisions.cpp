#include <cstddef>
#include <vector>

#include "collision_rules.hpp"
#include "particle_system.hpp"

namespace aos {

  // Merges every overlapping pair in one pass, checking pairs in index (= id) order.
  // An absorbed particle is marked with mass 0 and skipped; all of them are erased once at the end.
  // Same result as restarting the scan after every merge: a merge never moves the survivor or
  // changes its radius, so every pair checked earlier still does not overlap.
  void particle_system::process_collisions() {
    bool any_merged = false;
    for (std::size_t i = 0; i < particles_.size(); ++i) {
      for (std::size_t j = i + 1; j < particles_.size() and particles_.at(i).mass > 0.0; ++j) {
        particle & first  = particles_.at(i);
        particle & second = particles_.at(j);
        if (second.mass <= 0.0 or
            !common::checkcollision(first.pos, first.radius, second.pos, second.radius)) {
          continue;
        }
        bool const second_wins =
            second.mass > first.mass or
            (second.mass == first.mass and
             (second.radius > first.radius or
              (second.radius == first.radius and second.id < first.id)));
        if (second_wins) {
          common::merge_particles(second, first);
          first.mass = 0.0;   // first is gone: the loop condition ends this row
        } else {
          common::merge_particles(first, second);
          second.mass = 0.0;  // second is gone: keep checking first against the rest
        }
        any_merged = true;
      }
    }
    if (any_merged) {
      std::erase_if(particles_, [](particle const & p) { return p.mass <= 0.0; });
    }
  }

}  // namespace aos
