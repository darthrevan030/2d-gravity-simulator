#include <gtest/gtest/h>
#include "particle_system.hpp"

TEST(ParticleSystemTest, ConstructsCorrectly){
  std::vector<aos::particle> particles;
  aos::particle_system system{particles};
  EXPECT_TRUE(system.particles().empty());
}

TEST(ParticleSystemTest, SimulationRuns){
  common::particle_record p;
  p.mass=1.0;
  p.radius=1.0;
  p.pos = common::point{50.0, 50.0};
  p.vel = common::vec2d{0.0, 0.0};
  
  aos::particle_system system({p});
  system.simulate(10, 0.1, 100.0, 100.0);
  EXPECT_EQ(system.particles().size(), 1); 
}
