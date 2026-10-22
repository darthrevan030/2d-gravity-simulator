#include "particle_system.hpp"
#include <gtest/gtest.h>

namespace{
    // validity test uses box 100x50 box
    constexpr double box_width = 100.0;
    constexpr double box_height = 50.0;

    // build the value of one particle (velocity stays zero)
    soa::particle_values make_values(double mass, double radius, double x, double y) {
        soa::particle_values p;
        p.mass = mass;
        p.radius = radius;
        p.x = x;
        p.y = y;
        return p;
    }

}

// manual testing of is_valid_particle
TEST(test_particle_system, particle_in_the_middle_is_valid){
    EXPECT_TRUE(soa::is_valid_particle(make_values(10.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, zero_mass_is_invalid) {
    EXPECT_FALSE(soa::is_valid_particle(make_values(0.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, negative_mass_is_invalid) {
    EXPECT_FALSE(soa::is_valid_particle(make_values(-1.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, zero_radius_is_invalid){
    EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, 0.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, negative_radius_is_invalid){
    EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, -2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, x_exactly_on_both_limits_is_valid) {
    EXPECT_TRUE(soa::is_valid_particle(make_values(10.0, 2.0, 2.0, 25.0), box_width, box_height));
    EXPECT_TRUE(soa::is_valid_particle(make_values(10.0, 2.0, 98.0, 25.0), box_width, box_height));
}

TEST(test_particle_system, x_outside_limits_is_invalid){
    EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, 2.0, 1.9, 25.0), box_width, box_height));
    EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, 2.0, 98.1, 25.0), box_width, box_height));
}

TEST(test_particle_system, y_exactly_on_both_limits_is_valid){
    EXPECT_TRUE(soa::is_valid_particle(make_values(10.0, 2.0, 50.0, 2.0), box_width, box_height));
    EXPECT_TRUE(soa::is_valid_particle(make_values(10.0, 2.0, 50.0, 48.0), box_width, box_height));
}

TEST(test_particle_system, y_outside_limit_is_invalid) {
  EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, 2.0, 50.0, 1.9), box_width, box_height));
  EXPECT_FALSE(soa::is_valid_particle(make_values(10.0, 2.0, 50.0, 48.1), box_width, box_height));
}

// add_particle and particle_count
// these tests are for storage particles correctly 

// test 1: brand new system with no particles in it
TEST(test_particle_system, new_system_is_empty) {
    soa::particle_system const system;
    EXPECT_EQ(soa::particle_count(system), 0U); // U: unsigned, int:0; new system has zero particles
}

// test 2: add_particle puts each of the 6 input values into the right vector
TEST(test_particle_system, add_particle_puts_each_value_in_its_own_vector){
    soa::particle_system system; // empyt not const bc about to add to it
    soa::particle_values p = make_values(10.0, 2.0, 30.0, 40.0);
    p.vx = 5.0; // x velocity = 5
    p.vy = -6.0; // y velocity = -6

    soa:add_particle(system, p); // Add partile to system
    ASSERT_EQ(soa::particle_count(system), 1U); // must be exactly one particle, ASSERT stop the test if not 
    EXPECT_EQ(system.mass.front(), 10.0); // first value in the vector; mass = 10;
    EXPECT_EQ(system.radius.front(), 2.0); // vector holds radius=2
    EXPECT_EQ(system.x.front(), 30.0); // x pos 30
    EXPECT_EQ(system.y.front(), 40.0); // y pos 40
    EXPECT_EQ(system.vx.front(), 5.0); // x velocity=5
    EXPECT_EQ(system.vy.front(), -6.0); // y velocity = -6
    EXPECT_EQ(system.ax.front(), 0.0); 
    EXPECT_EQ(system.ay.front(), 0.0);
}

// test 3: particle stays in the order they were added aka position vetors=id
TEST(test_particle_system, add_particle_keeps_input_order){
    soa::particle_system system; 
    soa::add_particle(system, make_values(1.0, 1.0, 10.0, 10.0)); // first particle added: mass 1 --> id 0
    soa::add_particle(system, make_values(2.0, 1.0, 20.0, 20.0)); // second particle added: mass 2 --> id 1
    soa::add_particle(system, make_values(3.0, 1.0, 30.0, 30.0)); // first particlt added: mass 3 --> 2

    ASSERT_EQ(soa::particle_count(system), 3U); 
    EXPECT_EQ(system.mass.front(), 1.0); // first value in the vector is frist particle added (mass 1_)
    EXPECT_EQ(system.mass.back(), 3.0); // last value is last particle added at the end
    EXPECT_EQ(system.x.back(), 30.0); // check second vector, x is the third particle's x, 30

}

//Test 4: all 8 vectors are always the same length; growing and shrinking together
TEST(test_particle_system, all_vectors_always_have_the_same_length){
    // add all particles
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 1.0, 10.0, 10.0)); // particle 1
    soa::add_particle(system, make_values(2.0, 1.0, 20.0, 20.0)); // particle 2
 
    // each vector can hold 2 values
    EXPECT_EQ(system.mass.size(), 2U);   // .size() = how many values the vector holds; each vector must hold 2
    EXPECT_EQ(system.radius.size(), 2U);  
    EXPECT_EQ(system.x.size(), 2U);
    EXPECT_EQ(system.y.size(), 2U);
    EXPECT_EQ(system.vx.size(), 2U);
    EXPECT_EQ(system.vy.size(), 2U);
    EXPECT_EQ(system.ax.size(), 2U);
    EXPECT_EQ(system.ay.size(), 2U);
    /*
    if add_particle didn't push_back for some vector, that vector would hold fewer values for the others
    particle k would no longer line up across the vectors and the simulation would slightly mix up particles... to mitigate:
    this test catches this. acceleration must grow too, even though they start at 0 the 8th and last vector
    */
}