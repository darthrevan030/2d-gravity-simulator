#include "particle_system.hpp"
#include <gsl/gsl> // gsl::at, gsl::index
#include <cmath> // std::sqrt 

namespace soa{
    bool particles_collide(particle_system const & system, gsl::index i, gsl::index j) {
        // x particle i minus x of partle j: how far apart in x
        double const dx = gsl::at(system.x, i) - gsl::at(system.x, j);
        // particle y: how far apart in y
        double const dy = gsl::at(system.y, i) - gsl::at(system.y, j);

        // straight line distance between the centers (pythagorars)
        double const distance = std::sqrt(dx * dx + dy * dy);

        // the twp radii added together
        double const radii = gsl::at(system.radius, i) + gsl::at(system.radius, j);
        
        // overlap only if the centers are closer than the radii; exactly touching is not a collision
        return distance < radii;    
    }

    bool first_absorbs_second(particle_system const & system, gsl::index i, gsl::index j){
        // RULE 1: check masses
        double const mass_i = gsl::at(system.mass, i); // mass of particle i
        double const mass_j = gsl::at(system.mass, j); // mass or particle j
        if (mass_i > mass_j) {return true;} // if i is heavier, i wins
        if (mass_i < mass_j) {return false;} // if j is heavier j wins

        // RULE 2 (if masses are equal): check radius of particles, bigger wins
        double const radius_i = gsl::at(system.radius, i); // radius of particlce i
        double const radius_j = gsl::at(system.radius, j); // radius of particle j
        if(radius_i > radius_j) {return true;}
        if(radius_i < radius_j) {return false;}

        // RULE 3: smaller id wins, position=id order so if i is earlier than j 
        return i < j;
    }

    void absorb(particle_system & system, gsl::index absorber, gsl::index absorbed) { 
        // absorber's mass += absorbed mass
        gsl::at(system.mass, absorber) += gsl::at(system.mass, absorbed);
        gsl::at(system.vx, absorber) += gsl::at(system.vx, absorbed); // x vel add
        gsl::at(system.vy, absorber) += gsl::at(system.vy, absorbed); // y vel add
        // -- radius, x, y are not touched so the absorber keeps its own radius and position
    }
}