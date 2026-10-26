#include <gtest/gtest.h>

#include <cmath>

#include "particle_system.hpp"
#include "test_helpers.hpp"

using namespace test::make_particle;

TEST(IntegrateTest, SimulationDoesNotThrow){
    common::particle_record p;
    p.mass=1.0;
    p.radius=1.0;
    p.pos = common::point{10.0, 10.0};
    p.vel = common::vec2d{1.0, 1.0};
    aos::particle_system system({p});
    EXPECT_NO_THROW(system.simulate(1, 0.1, 100.0, 100.0));
}

TEST(IntegrateTest, EmptySimulation){
    aos::particle_system system({});
    EXPECT_NO_THROW(system.simulate(1, 0.1, 100.0, 100.0));
}
