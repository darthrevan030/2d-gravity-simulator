#include <cstddef>
#include <vector>

#include "particle_system.hpp"
#include "physics.hpp"

namespace aos {

std::vector<common::vec2d> particle_system::compute_accelerations() const {
  std::size_t const n = particles_.size();

  std::vector<common::vec2d> acc(n, common::vec2d{});

  for (std::size_t i = 0; i < n; ++i) {
    auto const & pi = particles_.at(i);

    for (std::size_t j = i + 1; j < n; ++j) {
      auto const & pj = particles_.at(j);

      auto const force =
          common::compute_force(
              pi.pos,
              pi.mass,
              pj.pos,
              pj.mass);

      acc.at(i) =
          acc.at(i) +
          force * (1.0 / pi.mass);

      acc.at(j) =
          acc.at(j) +
          force * (-1.0 / pj.mass);
    }
  }

  return acc;
}

}  // namespace aos
