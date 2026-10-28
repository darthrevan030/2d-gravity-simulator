#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

#include "collision_grid.hpp"
#include "collision_rules.hpp"
#include "particle_system.hpp"
#include "point.hpp"

namespace aos {

  namespace {

    // Merges an overlapping pair: the heavier particle absorbs the other; ties go to the larger
    // radius, then to the lower id. The absorbed one is marked with mass 0.
    void merge_pair(particle & first, particle & second) {
      bool const second_wins =
          second.mass > first.mass or
          (second.mass == first.mass and
           (second.radius > first.radius or
            (second.radius == first.radius and second.id < first.id)));
      if (second_wins) {
        common::merge_particles(second, first);
        first.mass = 0.0;
      } else {
        common::merge_particles(first, second);
        second.mass = 0.0;
      }
    }

  }  // namespace

  // Merges every overlapping pair in one pass, checking pairs in index (= id) order.
  // An absorbed particle is marked with mass 0 and skipped; all of them are erased once at the end.
  // Same result as restarting the scan after every merge: a merge never moves the survivor or
  // changes its radius, so every pair checked earlier still does not overlap.
  // Only the pairs the grid finds near each other are checked; nothing moves during the pass, so
  // one grid is valid for all of it.
  void particle_system::process_collisions() {
    if (particles_.size() < 2) { return; }
    std::vector<common::point> centres;
    centres.reserve(particles_.size());
    std::ranges::transform(particles_, std::back_inserter(centres), &particle::pos);
    double const max_radius = std::ranges::max(particles_, {}, &particle::radius).radius;
    common::collision_grid const grid{centres, max_radius};

    std::vector<std::size_t> later;
    bool any_merged = false;
    for (std::size_t i = 0; i < particles_.size(); ++i) {
      grid.later_neighbours(i, later);  // ascending, so the same order as checking every j > i
      for (std::size_t const j : later) {
        particle & first  = particles_.at(i);
        particle & second = particles_.at(j);
        if (first.mass <= 0.0) { break; }  // first is gone: the rest of its row is skipped
        if (second.mass <= 0.0 or
            !common::checkcollision(first.pos, first.radius, second.pos, second.radius)) {
          continue;
        }
        merge_pair(first, second);
        any_merged = true;
      }
    }
    if (any_merged) {
      std::erase_if(particles_, [](particle const & p) { return p.mass <= 0.0; });
    }
  }

}  // namespace aos
