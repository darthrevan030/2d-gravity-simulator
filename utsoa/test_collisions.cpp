#include "particle_system.hpp"
#include <gtest/gtest.h>

namespace{
    // helpers:
    // builds value of one particle - vel=0
    soa::particle_values make_values(double mass, double radius, double x, double y){
        soa::particle_values p; // start with all zeros
        // fill with the four given values
        p.mass = mass;
        p.radius = radius;
        p.x = x;
        p.y = y;
        return p; 
    }
}

// particles_collide
TEST(test_collisions, overlapping_particles_collide){
    soa::particle_system system; // empty system
    soa::add_particle(system, make_values(1.0, 2.5, 10.0, 10.0)); // particle 0: radius 2.5 at (10, 10)
    soa::add_particle(system, make_values(1.0, 2.6, 13.0, 14.0)); // particle 1: radius 2.6 at (13, 14)
    EXPECT_TRUE(soa::particles_collide(system, 0, 1)); // radii add up to 5.1 > distance 5 (they overlap)
}

TEST(test_collisions, exactly_touching_particles_do_not_collide) {
    soa::particle_system system; // empty system
    soa::add_particle(system, make_values(1.0, 2.5, 10.0, 10.0)); // particle 0: radius 2.5
    soa::add_particle(system, make_values(1.0, 2.5, 13.0, 14.0)); // particle 1: radius 2.5
    EXPECT_FALSE(soa::particles_collide(system, 0, 1)); // radii add up to 5: touching is not a collision
}

TEST(test_collisions, separated_particles_do_not_collide) {
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 2.0, 10.0, 10.0)); // partcile 0: radius 2.0
    soa::add_particle(system, make_values(1.0, 2.9, 13.0, 14.0)); // particle 1: radius 2.9
    EXPECT_FALSE(soa::particles_collide(system, 0, 1)); // radii add up to 4.9 < distance 5 (gap between them)
}

TEST(test_collisions, collision_check_works_in_both_orders) {
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 2.5, 10.0, 10.0)); // partcile 0: radius 2.5
    soa::add_particle(system, make_values(1.0, 2.6, 13.0, 14.0)); // partcile 1: radius 2.6  --overlapping--
    EXPECT_TRUE(soa::particles_collide(system, 0, 1));  // ensuring they overlap eachother
    EXPECT_TRUE(soa::particles_collide(system, 1, 0));  // ensuring they overlap eachother
}

// first_absorbes_second
TEST(test_collisions, heavier_particle_absorbs) {
  soa::particle_system system;
  soa::add_particle(system, make_values(5.0, 1.0, 10.0, 10.0));  // particle 0: heavy but small
  soa::add_particle(system, make_values(2.0, 9.0, 10.0, 10.0));  // particle 1: light but big (mass is checked first)
  EXPECT_TRUE(soa::first_absorbs_second(system, 0, 1)); // heavy particle 0 absorbs light particle 1
  EXPECT_FALSE(soa::first_absorbs_second(system, 1, 0)); // asked the other way round: light particle 1 does NOT absorb particle 0
}

TEST(test_collisions, same_mass_bigger_radius_absorbs) {
  soa::particle_system system;
  soa::add_particle(system, make_values(5.0, 1.0, 10.0, 10.0));  // particle 0: small
  soa::add_particle(system, make_values(5.0, 3.0, 10.0, 10.0));  // particle 1: same mass, bigger
  EXPECT_TRUE(soa::first_absorbs_second(system, 1, 0)); // bigger particle 1 absorbs smaller particle 0
  EXPECT_FALSE(soa::first_absorbs_second(system, 0, 1)); // smaller particle 0 does not absorb particle 1
}

TEST(test_collisions, full_tie_smaller_position_absorbs) {
  soa::particle_system system;
  soa::add_particle(system, make_values(5.0, 1.0, 10.0, 10.0));  // particle 0
  soa::add_particle(system, make_values(5.0, 1.0, 10.0, 10.0));  // particle 1: identical mass and radius
  EXPECT_TRUE(soa::first_absorbs_second(system, 0, 1));          // tie: smaller position (id 0) wins
  EXPECT_FALSE(soa::first_absorbs_second(system, 1, 0));         // id 1 loses
}


// absorb
TEST(test_collisions, absorb_adds_mass_and_velocity) {
  soa::particle_system system;
  soa::particle_values big = make_values(3.0, 2.0, 10.0, 20.0);   // will be particle 0
  big.vx = 1.0;                                                   // give it a velocity
  big.vy = 2.0;
  soa::particle_values small = make_values(1.0, 5.0, 50.0, 60.0);  // will be particle 1
  small.vx = 3.0;
  small.vy = -4.0;
  soa::add_particle(system, big);    // particle 0
  soa::add_particle(system, small);  // particle 1

  soa::absorb(system, 0, 1);  // particle 0 absorbs particle 1

  EXPECT_EQ(system.mass.front(), 4.0);  // 3 + 1
  EXPECT_EQ(system.vx.front(), 4.0);    // 1 + 3
  EXPECT_EQ(system.vy.front(), -2.0);   // 2 + -4
}

TEST(test_collisions, absorb_keeps_absorber_radius_and_position) {
  soa::particle_system system;
  soa::add_particle(system, make_values(3.0, 2.0, 10.0, 20.0));  // particle 0: radius 2 at (10, 20)
  soa::add_particle(system, make_values(1.0, 5.0, 50.0, 60.0));  // particle 1: radius 5 at (50, 60)

  soa::absorb(system, 0, 1);  // 0 absorbs 1

  EXPECT_EQ(system.radius.front(), 2.0);  // still 2, not 5
  EXPECT_EQ(system.x.front(), 10.0);      // still 10, not 50
  EXPECT_EQ(system.y.front(), 20.0);      // still 20, not 60
}

TEST(test_collisions, absorb_works_when_later_particle_absorbs) {
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 1.0, 10.0, 10.0)); // particle 0: light
    soa::add_particle(system, make_values(4.0, 1.0, 11.0, 10.0)); // particle 1: heavy

    soa::absorb(system, 1, 0); // particle 1 absorbed particle 0 (the later one wins)
    EXPECT_EQ(system.mass.back(), 5.0); // particle 1 grew: 4+1
    EXPECT_EQ(system.x.back(), 11.0); // keep its own position
    EXPECT_EQ(system.mass.front(), 1.0); // particle 0 is untouched, gets removed later in the scan
}

TEST(test_collisions, absorb_does_not_remove_anything) {
    soa::particle_system system;
    soa::add_particle(system, make_values(3.0, 2.0, 10.0, 10.0)); // particle 0
    soa::add_particle(system, make_values(1.0, 2.0, 11.0, 10.0)); // particle 1

    soa::absorb(system, 0, 1); // particle 0 absorbs 1

    EXPECT_EQ(soa::particle_count(system), 2U); // still 2: removed later when we remove from all 8 vectors at once
    EXPECT_EQ(system.vy.size(), 2U); // vectors are all still the same length
}