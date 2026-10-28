#include "collision_grid.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <span>
#include <vector>

namespace common {

  namespace {
    // At most about this many cells per particle, so building and clearing the grid stays cheap
    // when the particles are spread out.
    constexpr double max_cells_per_particle = 4.0;
  }  // namespace

  collision_grid::collision_grid(std::span<point const> centres, double max_radius) {
    if (centres.empty()) { return; }
    auto const [left, right] = std::ranges::minmax(centres, {}, &point::x);
    auto const [bottom, top] = std::ranges::minmax(centres, {}, &point::y);
    min_x_ = left.x;
    min_y_ = bottom.y;
    double const width = right.x - left.x;
    double const height = top.y - bottom.y;
    double const cells_per_axis =
        std::max(1.0, std::sqrt(max_cells_per_particle * static_cast<double>(centres.size())));
    cell_ = std::max({2.0 * max_radius, std::max(width, height) / cells_per_axis,
                      std::numeric_limits<double>::min()});
    cols_ = static_cast<std::size_t>(width / cell_) + 1;
    rows_ = static_cast<std::size_t>(height / cell_) + 1;

    // counting sort of the particles by cell; stable, so indices stay ascending inside a cell
    cell_of_.resize(centres.size());
    std::ranges::transform(centres, cell_of_.begin(), [this](point const & p) { return cell_index(p); });
    start_.assign(cols_ * rows_ + 1, 0);
    for (std::size_t const cell : cell_of_) { ++start_.at(cell + 1); }
    std::partial_sum(start_.begin(), start_.end(), start_.begin());
    std::vector<std::size_t> next(start_.begin(), std::prev(start_.end()));
    members_.resize(centres.size());
    for (std::size_t k = 0; k < cell_of_.size(); ++k) {
      std::size_t & slot = next.at(cell_of_.at(k));
      members_.at(slot) = k;
      ++slot;
    }
  }

  void collision_grid::later_neighbours(std::size_t i, std::vector<std::size_t> & out) const {
    out.clear();
    std::size_t const row = cell_of_.at(i) / cols_;
    std::size_t const col = cell_of_.at(i) % cols_;
    for (std::size_t r = (row == 0) ? 0 : row - 1; r <= std::min(row + 1, rows_ - 1); ++r) {
      for (std::size_t c = (col == 0) ? 0 : col - 1; c <= std::min(col + 1, cols_ - 1); ++c) {
        std::size_t const cell = r * cols_ + c;
        auto const in_cell = std::span{members_}.subspan(start_.at(cell), start_.at(cell + 1) - start_.at(cell));
        std::ranges::copy_if(in_cell, std::back_inserter(out), [i](std::size_t j) { return j > i; });
      }
    }
    std::ranges::sort(out);
  }

  std::size_t collision_grid::cell_index(point const & p) const {
    std::size_t const col = std::min(static_cast<std::size_t>((p.x - min_x_) / cell_), cols_ - 1);
    std::size_t const row = std::min(static_cast<std::size_t>((p.y - min_y_) / cell_), rows_ - 1);
    return row * cols_ + col;
  }

}  // namespace common
