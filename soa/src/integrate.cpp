#include "particle_system.hpp"
#include <cmath> // abs value
#include <ranges>  // std::views::zip get several vectors side by side
#include <vector> // std::vector

namespace soa { 
    namespace { // ai reccomended a namespace helper
        
        // rebound alone one direction (x or y) for every particle 
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

    }

    void bounce_off_walls(particle_system & system, double width, double height) {
        bounce_one_axis(system.x, system.vx, system.radius, width); // left and right walls: x positions, x velocities
        bounce_one_axis(system.y, system.vy, system.radius, height); // bottom and top walls: y positions, y velocities

    }
}