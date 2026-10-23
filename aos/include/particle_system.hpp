#include <cstddef>
#include <vector>

#include "particle.hpp"
#include "vector.hpp"

namespace aos{
class particle_system{
public:
    explicit particle_system(std::vector<particle> particles);
    void simulate(
      std::size_t iterations;
      double dt;
      double width;
      double height);
    const std::vector<particle>& particles() const noexcept;

private:
    std::vector<particle> particles_;
    std::vector<common::vec2d> compute_accelerations() const;
    void process_collisions();
};
}
