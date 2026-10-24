#ifndef SOA_PARTICLE_SYSTEM_HPP
#define SOA_PARTICLE_SYSTEM_HPP

#include <cstddef>
#include <vector>
#include <gsl/gsl> // gives gsl::at (safe "x[i] and gsl::index - the type for positions")


namespace soa {
    // values of one particle exaclty as it appears on one line of the input file
    // used to check a particle before it is added to the log
    struct particle_values{
        double mass = 0.0; // m
        double radius = 0.0; // r
        double x = 0.0; // x coordinate position
        double y = 0.0; // y coordinate position
        double vx = 0.0; // volocity of x component
        double vy = 0.0; // velocity of y component
    };

    // one vectory per property - all particles stored as a "stucture of arrays"
    // particle number k is as position k in every vector and k is id so every vector must always have the same length
    struct particle_system{
        std::vector<double> mass; // mass[k] = mass of particle k
        std::vector<double> radius; // radius[k] = radius of particle k
        std::vector<double> x; // position of x coords
        std::vector<double> y; // position of y coords
        std::vector<double> vx; // velocity of x components
        std::vector<double> vy; // velocity of y components
        std::vector<double> ax; // accelortation of x components
        std::vector<double> ay; // acceleration of y components
    };

    // function for checking: mass and radius must be positive, the whole particle must fit inside the box of given width and height
    [[nodiscard]] bool is_valid_particle(particle_values const & p, double width, double height);

    // append particle to end of every vector, starts: a=0
    void add_particle(particle_system & system, particle_values const & p);

    // # of particles in system
    [[nodiscard]] std::size_t particle_count(particle_system const & system);

    // true if particles i and j overlap: their distance is less than the sum of their radii
    // returns yes/no, read-only access to all particles position of partcile 1, position of particle 2
    [[nodiscard]] bool particles_collide(particle_system const & system, gsl::index i, gsl::index j);
    
    // true if: when particles i and j collide, particle i absorbs particle j
    // rules in order: greater mass wins --> greater radius --> smaller positions (==smaller id)
    [[nodiscard]] bool first_absorbs_second(particle_system const & system, gsl::index i, gsl::index j);

    // merges particles: absorbed into absorber: add masses, add velocities
    // absorber keeps its own radius and position, absorbed particle removed later
    // not const: absorb changes the absorber's mass and velocity
    void absorb(particle_system & system, gsl::index absorber, gsl::index absorbed);
    
}
#endif