#include <gtest/gtest.h>

#include "collision_rules.hpp"
#include "point.hpp"
TEST(CollisionRules, CollsionDetection) {
  common::point  const p1{0.0, 0.0};
  common::point const  p2{1.0, 1.0};
  double const r1 = 1.0;
  double const r2 = 1.0;

  EXPECT_TRUE(common::checkcollision(p1, r1, p2, r2));

}
TEST(CollisionRules, NoCollisionDetection) {
  common::point const p1{0.0, 0.0};
  common::point const p2{3.0, 4.0};
  double const r1 = 1.0;
  double const r2 = 1.0;

  EXPECT_FALSE(common::checkcollision(p1, r1, p2, r2));

}
TEST(CollisionRules, MergeParticles) {
  common::particle_record  absorber{2.0, 1.0, {0.0, 0.0}, {1.0, 1.0}};
  common::particle_record  const absorbed{3.0, 2.0, {1.0, 1.0}, {2.0, 2.0}};

  common::merge_particles( absorber, absorbed);

  EXPECT_DOUBLE_EQ(absorber.mass, 5.0);
  EXPECT_DOUBLE_EQ(absorber.radius, 1.0);  
  EXPECT_DOUBLE_EQ(absorber.pos.getx(), 0.0);
  EXPECT_DOUBLE_EQ(absorber.pos.gety(), 0.0);
  EXPECT_DOUBLE_EQ(absorber.vel.getx(), 3.0);
  EXPECT_DOUBLE_EQ(absorber.vel.gety(), 3.0);
}