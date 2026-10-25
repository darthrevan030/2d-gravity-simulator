#include <gtest/gtest.h>

#include "particle_system.hpp"

TEST(IntegrateTest, EmptySystem)
{
    std::vector<aos::particle> particles;
    aos::particle_system system{particles};

    EXPECT_TRUE(system.particles().empty());
}
