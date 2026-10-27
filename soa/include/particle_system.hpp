#ifndef SOA_PARTICLE_SYSTEM_HPP
#define SOA_PARTICLE_SYSTEM_HPP

#include <cstddef>
#include <vector>
#include <gsl/gsl> // gives gsl::at (safe "x[i] and gsl::index - the type for positions")
#include <istream> // input read from files
#include <ostream> // output is written to 



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
        std::vector<std::size_t> id; // original id of each particle, printed in the output files
        std::vector<double> mass; // mass[k] = mass of particle k
        std::vector<double> radius; // radius[k] = radius of particle k
        std::vector<double> x; // position of x coords
        std::vector<double> y; // position of y coords
        std::vector<double> vx; // velocity of x components
        std::vector<double> vy; // velocity of y components
        std::vector<double> ax; // accelortation of x components
        std::vector<double> ay; // acceleration of y components
    };

    // four command line values together (clang-tidy allows 4 param per function so these will be one)
    struct simulation_settings{
        std::size_t iterations = 0; // how many time steps to simulate
        double width = 0.0; // box width - right
        double height = 0.0; // box height - top
        double time_step = 0.0; // dt
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

    // removes every particle whose mass was set to 0 (absorbed) from ALL vectors, keeping the order
    void remove_absorbed(particle_system & system);

    // checks every pair in id order, merges the ones that overlap, then removes the absorbed particles
    void resolve_collisions(particle_system & system);
    
    // motion: for every particle: if it touches or crosses a wall, put it back on that wall and make its velocity point
    // into the box (rebound effect)
    void bounce_off_walls(particle_system & system, double width, double height);

    // for every particle: new velocity v=v+a*dt; new position p=p+v*dt
    void move_particles(particle_system & system, double time_step);

    // gravity: sets every particles acceleration from the pull of all the others
    void compute_accelerations(particle_system & system);

    // one full iteration: gravity --> velocity and position --> walls --> collisions
    void simulate_step(particle_system & system, simulation_settings const & settings);

    // run settings.iterations step, one after another
    void run_simulation(particle_system & system, simulation_settings const & settings);

    // reads particles line by line from the input; malformed lines are ignored, invalid particles discarded
    [[nodiscard]] particle_system read_particles(std::istream & input, double width, double height);

    // writes the header line and one "id: m r x y vx vy" line per particle, all with 3 decimals
    void write_particles(std::ostream & output, particle_system const & system, simulation_settings const & settings);
    
}
#endif


