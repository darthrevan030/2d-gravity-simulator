#include "particle_system.hpp"

#include <cstddef>

namespace soa {

    
    bool is_valid_particle(particle_values const & p, double width, double height){ // all four must hold
        bool const positive_mass = p.mass > 0.0; // positive mass
        bool const positive_radius = p.radius > 0.0; // positive radius

        // whole particle must fit insode the box - center must be at least 1 radius away from every wall 
        bool const x_inside = p.x >= p.radius and p.x <= width - p.radius; 
        bool const y_inside = p.y >= p.radius and p.y <= height - p.radius;

        return positive_mass and positive_radius and x_inside and y_inside;
    }

    void add_particle(particle_system & system, particle_values const & p){
        system.id.push_back(particle_count(system)); // id=particle amount before this one (0,1,2...)
        system.mass.push_back(p.mass); // push_back adds one vaue at the end of the vector
        system.radius.push_back(p.radius);
        system.x.push_back(p.x);
        system.y.push_back(p.y);
        system.vx.push_back(p.vx);
        system.vy.push_back(p.vy);
        system.ax.push_back(0.0); // accelerations computed at every step
        system.ay.push_back(0.0);
    }

    std::size_t particle_count(particle_system const & system) {
        return system.mass.size(); // every vector has same legnth 
    }
}
