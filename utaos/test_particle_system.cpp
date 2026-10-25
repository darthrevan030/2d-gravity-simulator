#include <gtest/gtest/h>
#include "particle_system.hpp"

TEST(ParticleSystemTest, ConstructsCorrectly){
  std::vector<aos::particle> particles;
  aos::particle_system system{particles};
  EXPECT_TRUE(system.particles().empty());
}
