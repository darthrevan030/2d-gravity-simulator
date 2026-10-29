#include "particle_system.hpp"

#include <cmath> // abs value
#include <gsl/gsl>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>  // std::views::zip get several vectors side by side
#include <vector> // std::vector

namespace soa { 
    namespace { // ai reccomended a namespace helper
        constexpr double gravitational_constant = 6.674e-11;
    
        // wall rebound alone one direction (x or y) for every particle 
        void bounce_one_axis(std::vector<double> & position, std::vector<double> & velocity, std::vector<double> const & radius, double limit){
            // positions to fix: x or y; velocity to fix: vx or vy; radii: only read; limit is far wall
            for (auto && [pos, vel, rad] : std::views::zip(position, velocity, radius)){
                // pos, vel, and rad is one particles position, velocity, and radius
                if(pos-rad <= 0.0){  // left/bottom edge of the circle at or past 0: touching or past the near wall
                    pos = rad; // parctile exactly on wall 
                    vel = std::abs(vel); // make velocity positive --> moving away from the far wall
                }

                if(pos + rad >= limit){ // right/top at or past the far wall: touching or past it
                    pos = limit - rad; // particle exactly on far wall
                    vel = -std::abs(vel); // velocity negatice --> moving away from the far wall
                }
            }
        }
        // velocity then position update along one direcetion for every particle
        void move_one_axis(std::vector<double> & position, std::vector<double> & velocity, std::vector<double> const & acceleration, double time_step) {
            // positions change (x/y), velocities change(vx/vy), acceleration only read
            for (auto && [pos, vel, acc] : std::views::zip(position, velocity, acceleration)) {
                // changing real values in the vectors, writing to them changes the vectors
                vel = vel + acc * time_step;
                pos = pos + vel * time_step;
            }
        } // velocity first so when the position line runs, vel holds new velocity, otherwise the positions would use old velocity

        // adds the gravity btwn partile i and every later particle j:
        // +F to particle i and -F to partile j (equal and opposite) so each pair is done once
        // performance: i's sum is kept in locals (a reference into ax would make the compiler reload it
        // every pair, since ax[j] is written in the same loop), and the later particles are walked with
        // plain iterators, which g++ vectorizes (a zip | drop view it does not)
        void add_forces_with_later_particles(particle_system & system, gsl::index i){
            double const xi = gsl::at(system.x, i);     // particle i's values, read once
            double const yi = gsl::at(system.y, i);
            double const mi = gsl::at(system.mass, i);
            double fxi      = 0.0;                      // particle i's force from the later particles
            double fyi      = 0.0;
            auto const first = static_cast<std::ptrdiff_t>(i + 1); // only particles after i
            auto xj  = system.x.cbegin() + first;
            auto yj  = system.y.cbegin() + first;
            auto mj  = system.mass.cbegin() + first;
            auto fxj = system.ax.begin() + first;
            auto fyj = system.ay.begin() + first;
            for (auto const x_end = system.x.cend(); xj != x_end; ++xj, ++yj, ++mj, ++fxj, ++fyj) {
                double const dx         = *xj - xi;                      // p_j - p_i
                double const dy         = *yj - yi;
                double const dist_sq    = dx * dx + dy * dy;             // |p_j - p_i|^2
                double const dist_cubed = dist_sq * std::sqrt(dist_sq);  // |p_j - p_i|^3, no pow()
                double const scale      = gravitational_constant * mi * *mj / dist_cubed; // one division per pair
                double const fx         = scale * dx;                    // G mi mj (p_j - p_i) / |p_j - p_i|^3
                double const fy         = scale * dy;
                fxi += fx;  // i is pulled toward j
                fyi += fy;
                *fxj -= fx;  // j is pulled toward i, same size, opposite direction
                *fyj -= fy;
            }
            gsl::at(system.ax, i) += fxi; // add to what the earlier particles already gave i
            gsl::at(system.ay, i) += fyi;
        }

    } // namespace

    void bounce_off_walls(particle_system & system, double width, double height) {
        bounce_one_axis(system.x, system.vx, system.radius, width); // left and right walls: x positions, x velocities
        bounce_one_axis(system.y, system.vy, system.radius, height); // bottom and top walls: y positions, y velocities
    }

    void move_particles(particle_system & system, double time_step){
        move_one_axis(system.x, system.vx, system.ax, time_step); // x direction: x, vx, ax
        move_one_axis(system.y, system.vy, system.ay, time_step); // y direction: y, vy, ay
    }

    void compute_accelerations(particle_system & system) {
        std::ranges::fill(system.ax, 0.0); // 1. start from zero (forget last step's values)
        std::ranges::fill(system.ay, 0.0);
        gsl::index const n = std::ssize(system.mass);
        for (gsl::index i = 0; i < n; ++i) {  // 2. every pair (i, j) with i < j, exactly once
            add_forces_with_later_particles(system, i);
        }
        for (auto && [fx, fy, m] : std::views::zip(system.ax, system.ay, system.mass)) {
            fx = fx / m;  // 3. a = F / m (ax held the force sum until now)
            fy = fy / m;
        }
    }

    void simulate_step(particle_system & system, simulation_settings const & settings) {
        compute_accelerations(system); // 1. gravity: every particle's acceleration
        move_particles(system, settings.time_step); // 2. v = v + a*dt, then p = p + v*dt
        bounce_off_walls(system, settings.width, settings.height); // 3. push back anything that hit a wall
        resolve_collisions(system); // 4. merge overlapping particles
    }

    void run_simulation(particle_system & system, simulation_settings const & settings) {
        for (std::size_t step = 0; step < settings.iterations; ++step) { // repeat `iterations` times
            simulate_step(system, settings);
        }
    }

} // soa namespace