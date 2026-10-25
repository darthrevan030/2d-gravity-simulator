#include <gtest/gtest.h>
#include "particle_system.hpp"

TEST(CollisionTest, LargerMassAbsorbs){
  common::particle_record a;
  common::particle_record b;

  a.mass = 10.0;
  a.radius = 2.0;
  a.pos={0.0, 0.0};

  b.mass = 5.0;
  b.radius = 2.0;
  b.pos={1.0, 0.0};
  aos::particle_system system({a, b});
  system.simulate(1, 0.0, 1000.0, 1000.0);
  EXPECT_EQ(system.particles().size(), 1);
}

TEST(CollisionTest, EqualMassLargerRadiusWins){
  common::particle_record a;
  common::particle_record b;

  a.mass = 10.0;
  a.radius = 1.0;
  a.pos={0.0, 0.0};

  b.mass = 10.0;
  b.radius = 2.0;
  b.pos={1.0, 0.0};
  aos::particle_system system({a, b});
  system.simulate(1, 0.0, 1000.0, 1000.0);
  EXPECT_EQ(system.particles().size(), 1);
}


  
