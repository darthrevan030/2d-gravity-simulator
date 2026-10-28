#ifndef TESTUTIL_REFERENCE_SIM_HPP
#define TESTUTIL_REFERENCE_SIM_HPP

// Naive reference simulator used as an oracle by the aos and soa equivalence tests.
//
// It is written to be obviously correct rather than fast, and on purpose shares no code with
// the production implementation (not even the constants or the physics helpers), so a bug in
// production code cannot be hidden by the same bug in the reference:
//   - plain struct of doubles, no common:: types
//   - every particle sums the pull of every *other* particle (full n^2, no symmetry shortcut)
//   - collisions: repeatedly find the first overlapping pair in index order and merge it
//
// Rules implemented (from ftest/cases):
//   1. merge overlapping particles (also before the first iteration)
//   2. per iteration: accelerations -> velocity update -> position update -> wall rebound
//      -> merge overlapping particles
//   3. a merge keeps the absorber's position and radius, adds mass and velocity; the heavier
//      particle absorbs, ties go to the larger radius, then to the lower index

#include <gsl/gsl>

#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace reference {

  inline constexpr double gravity = 6.674e-11;
  inline constexpr double epsilon = 1e-9;

  struct body {
    double mass;
    double radius;
    double x;
    double y;
    double vx;
    double vy;
  };

  [[nodiscard]] inline bool overlap(body const & a, body const & b) {
    double const dist = std::hypot(a.x - b.x, a.y - b.y);
    return a.radius + b.radius > dist;
  }

  [[nodiscard]] inline std::optional<std::pair<std::size_t, std::size_t>>
  first_overlapping_pair(std::vector<body> const & bodies) {
    for (std::size_t i = 0; i < bodies.size(); ++i) {
      for (std::size_t j = i + 1; j < bodies.size(); ++j) {
        if (overlap(bodies.at(i), bodies.at(j))) { return std::make_pair(i, j); }
      }
    }
    return std::nullopt;
  }

  inline void merge_all(std::vector<body> & bodies) {
    while (auto const pair = first_overlapping_pair(bodies)) {
      auto const [i, j] = *pair;
      bool const j_wins = bodies.at(j).mass > bodies.at(i).mass or
                          (bodies.at(j).mass == bodies.at(i).mass and bodies.at(j).radius > bodies.at(i).radius);
      std::size_t const winner = j_wins ? j : i;
      std::size_t const loser = j_wins ? i : j;
      bodies.at(winner).mass += bodies.at(loser).mass;
      bodies.at(winner).vx += bodies.at(loser).vx;
      bodies.at(winner).vy += bodies.at(loser).vy;
      bodies.erase(bodies.begin() + gsl::narrow<std::ptrdiff_t>(loser));
    }
  }

  inline void bounce(double & pos, double & vel, double radius, double limit) {
    if (pos - radius <= 0.0) {
      pos = radius;
      if (vel < 0.0) { vel = -vel; }
    }
    if (pos + radius >= limit) {
      pos = limit - radius;
      if (vel > 0.0) { vel = -vel; }
    }
  }

  inline void step(std::vector<body> & bodies, double dt, double width, double height) {
    std::vector<double> ax(bodies.size(), 0.0);
    std::vector<double> ay(bodies.size(), 0.0);
    for (std::size_t i = 0; i < bodies.size(); ++i) {
      for (std::size_t j = 0; j < bodies.size(); ++j) {
        if (i == j) { continue; }
        double const dx = bodies.at(j).x - bodies.at(i).x;
        double const dy = bodies.at(j).y - bodies.at(i).y;
        double const dist_sq = dx * dx + dy * dy;
        if (dist_sq < epsilon * epsilon) { continue; }
        double const dist = std::sqrt(dist_sq);
        double const pull = gravity * bodies.at(j).mass / (dist * dist * dist);
        ax.at(i) += pull * dx;
        ay.at(i) += pull * dy;
      }
    }
    for (std::size_t i = 0; i < bodies.size(); ++i) {
      bodies.at(i).vx += ax.at(i) * dt;
      bodies.at(i).vy += ay.at(i) * dt;
      bodies.at(i).x += bodies.at(i).vx * dt;
      bodies.at(i).y += bodies.at(i).vy * dt;
      bounce(bodies.at(i).x, bodies.at(i).vx, bodies.at(i).radius, width);
      bounce(bodies.at(i).y, bodies.at(i).vy, bodies.at(i).radius, height);
    }
    merge_all(bodies);
  }

  struct run_settings {
    std::size_t iterations;
    double dt;
    double width;
    double height;
  };

  [[nodiscard]] inline std::vector<body> simulate(std::vector<body> bodies, run_settings const & run) {
    merge_all(bodies);
    for (std::size_t it = 0; it < run.iterations; ++it) { step(bodies, run.dt, run.width, run.height); }
    return bodies;
  }

}  // namespace reference

#endif
