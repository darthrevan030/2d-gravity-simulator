#ifndef TESTUTIL_SCENARIOS_HPP
#define TESTUTIL_SCENARIOS_HPP

// Seeded random scenario generator shared by the aos and soa equivalence tests.

#include <cstddef>
#include <random>
#include <vector>

#include "reference_sim.hpp"

namespace testutil {

  struct scenario {
    std::vector<reference::body> bodies;
    std::size_t iterations;
    double dt;
    double width;
    double height;
  };

  // Particles are placed so that they lie fully inside the world. A smaller world with the same
  // particle count gives a denser scenario and therefore more collisions. The time step is 0.1;
  // callers that need another one overwrite `dt` on the result.
  [[nodiscard]] inline scenario make_scenario(unsigned seed, std::size_t count, double world,
                                              std::size_t iterations) {
    double const dt = 0.1;
    std::mt19937 gen{seed};
    std::uniform_real_distribution<double> mass{1e10, 1e13};
    std::uniform_real_distribution<double> radius{1.0, 5.0};
    std::uniform_real_distribution<double> vel{-5.0, 5.0};
    std::uniform_real_distribution<double> pos{5.0, world - 5.0};

    scenario s{{}, iterations, dt, world, world};
    for (std::size_t i = 0; i < count; ++i) {
      double const m = mass(gen);
      double const r = radius(gen);
      double const x = pos(gen);
      double const y = pos(gen);
      double const vx = vel(gen);
      double const vy = vel(gen);
      s.bodies.push_back({m, r, x, y, vx, vy});
    }
    return s;
  }

}  // namespace testutil

#endif
