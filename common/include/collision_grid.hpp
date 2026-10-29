#ifndef COMMON_COLLISION_GRID_HPP
#define COMMON_COLLISION_GRID_HPP

#include <cstddef>
#include <span>
#include <vector>

#include "point.hpp"

namespace common {

  // Uniform grid over the particle centres, used to find collision candidates without checking
  // every pair. Two particles can only overlap if their centres are closer than r_i + r_j, which
  // is at most twice the largest radius, so with cells at least that big every overlapping pair
  // lies in the same cell or in neighbouring ones.
  class collision_grid {
  public:
    collision_grid(std::span<point const> centres, double max_radius);

    // Writes into `out` every particle j > i in i's cell or the 8 around it, in ascending order.
    // This includes every later particle that can overlap i (and some that don't).
    void later_neighbours(std::size_t i, std::vector<std::size_t> & out) const;

  private:
    [[nodiscard]] std::size_t cell_index(point const & p) const;

    double min_x_ = 0.0;
    double min_y_ = 0.0;
    double cell_ = 1.0;
    std::size_t cols_ = 1;
    std::size_t rows_ = 1;
    std::vector<std::size_t> start_;    // cell c holds members_[start_[c]] .. members_[start_[c + 1] - 1]
    std::vector<std::size_t> members_;  // particle indices sorted by cell, ascending inside a cell
    std::vector<std::size_t> cell_of_;  // cell of each particle
  };

}  // namespace common

#endif
