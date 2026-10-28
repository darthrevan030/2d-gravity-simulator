#include <cstddef>
#include <vector>

#include "particle_system.hpp"
#include "physics.hpp"

namespace aos {

  std::vector<common::vec2d> particle_system::compute_accelerations() const {
    std::vector<common::vec2d> acc(particles_.size(), common::vec2d{});
    for (std::size_t i = 0; i < particles_.size(); ++i) {
      for (std::size_t j = i + 1; j < particles_.size(); ++j) {
        // force on i points towards j; j feels the opposite
        auto const force = common::compute_force(particles_.at(i).pos, particles_.at(i).mass,
                                                 particles_.at(j).pos, particles_.at(j).mass);
        acc.at(i) = acc.at(i) + force * (1.0 / particles_.at(i).mass);
        acc.at(j) = acc.at(j) + force * (-1.0 / particles_.at(j).mass);
      }
    }
    return acc;
  }

}  // namespace aos
