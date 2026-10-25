#include <gtest/gtest.h>

#include "particle_system.hpp"

TEST(IntegrateTest, EmptySystem)
{
    std::vector<aos::particle> particles;
    aos::particle_system system{particles};

    EXPECT_TRUE(system.particles().empty());
}

TEST(IntegrateTest, SingleParticleRemains)
{
    common::particle_record p;

    p.mass=1.0;
    p.radius=1.0;
    std::vector<aos::particle> particles{p};
    aos::particle_system system{particles};
    system.simulate(1, 0.1, 1000.0, 1000.0);
    EXPECT_EQ(system.particles().size(), 1);
}
