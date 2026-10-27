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
        void add_forces_with_later_particles(particle_system & system, gsl::index i){
            double const xi = gsl::at(system.x, i);     // particle i's values, read once
            double const yi = gsl::at(system.y, i);
            double const mi = gsl::at(system.mass, i);
            double & fxi    = gsl::at(system.ax, i);    // particle i's running force sum (a reference)
            double & fyi    = gsl::at(system.ay, i);
            auto later = std::views::zip(system.x, system.y, system.mass, system.ax, system.ay) |
                        std::views::drop(i + 1);       // only particles after i
            for (auto && [xj, yj, mj, fxj, fyj] : later) {
                double const dx         = xj - xi;                       // p_j - p_i
                double const dy         = yj - yi;
                double const dist_sq    = dx * dx + dy * dy;             // |p_j - p_i|^2
                double const dist_cubed = dist_sq * std::sqrt(dist_sq);  // |p_j - p_i|^3, no pow()
                double const g_masses   = gravitational_constant * mi * mj;
                double const fx         = g_masses * dx / dist_cubed;    // G mi mj (p_j - p_i) / |p_j - p_i|^3
                double const fy         = g_masses * dy / dist_cubed;
                fxi += fx;  // i is pulled toward j
                fyi += fy;
                fxj -= fx;  // j is pulled toward i, same size, opposite direction
                fyj -= fy;
            }

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

} // soa namespace