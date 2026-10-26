#include "particle_system.hpp"

#include "physics.hpp"

#include <utility>

namespace aos {

particle_system::particle_system(
    std::vector<particle> particles)
    : particles_(std::move(particles))
{
    process_collisions();
}

const std::vector<particle>&
particle_system::particles() const noexcept
{
    return particles_;
}

void particle_system::simulate(
    std::size_t iterations,
    double dt,
    double width,
    double height)
{
    for (std::size_t step = 0; step < iterations; ++step)
    {
        auto accelerations = compute_accelerations();

        for (std::size_t i = 0; i < particles_.size(); ++i)
        {
            common::update_kinematics(
                particles_[i].pos,
                particles_[i].vel,
                accelerations[i],
                dt);

            common::apply_rebound(
                particles_[i].pos,
                particles_[i].vel,
                particles_[i].radius,
                width,
                height);
        }

        process_collisions();
    }
}

}
