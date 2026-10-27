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
    common::enclosure const box{width, height};

    for (std::size_t step = 0; step < iterations; ++step)
    {
        auto accelerations = compute_accelerations();

        std::size_t const n = particles_.size();

        for (std::size_t i = 0; i < n; ++i)
        {
            auto & p = particles_.at(i);

            common::update_kinematics(
                p.pos,
                p.vel,
                accelerations.at(i),
                dt);

            common::apply_rebound(
                p.pos,
                p.vel,
                p.radius,
                box);
        }

        process_collisions();
    }
}

}
