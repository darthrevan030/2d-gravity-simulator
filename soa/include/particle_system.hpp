#ifndef SOA_PARTICLE_SYSTEM_HPP
#define SOA_PARTICLE_SYSTEM_HPP

#include <cstddef>
#include <vector>

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

}
#endif