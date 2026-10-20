#include "particle.hpp"
#include <gtest/gtest.h> // CMake

namespace{
    constexpr double box_width = 100; // 100x50 box
    constexpr double box_height = 50; // 100x50 box

    // builds particle with given mass, radius and position - vel and acc stay 0 rn
    aos::particle make_particle(double mass, double radius, double x, double y){
        aos::particle p;
        p.mass = mass;
        p.radius = radius;
        p.x = x;
        p.y = y;
        return p;
    }
}

// utaos our unit tests: fixed variables written to test the file, must call is_valid_partciel with known cases, PASSED/FAILED each test
TEST(test_particle, new_particle_starts_at_zero_and_alive){
    aos::particle const p;
    EXPECT_EQ(p.id, 0U);
    EXPECT_EQ(p.mass, 0.0);
    EXPECT_EQ(p.ax, 0.0);
    EXPECT_TRUE(p.alive);
}

TEST(test_particle, particle_in_the_middle_is_valid){
    EXPECT_TRUE(aos::is_valid_particle(make_particle(10.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle, zero_mass_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(0.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle, negative_mass_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(-1.0, 2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle, zero_radius_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, 0.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle, negative_radius_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, -2.0, 50.0, 25.0), box_width, box_height));
}

TEST(test_particle, x_exactly_on_left_limit_is_valid){
    EXPECT_TRUE(aos::is_valid_particle(make_particle(10.0, 2.0, 2.0, 25.0), box_width, box_height));
}

TEST(test_particle, x_past_left_limit_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, 2.0, 1.9, 25.0), box_width, box_height));
}

TEST(test_particle, x_exactly_on_right_limit_is_valid){
    EXPECT_TRUE(aos::is_valid_particle(make_particle(10.0, 2.0, 98.0, 25.0), box_width, box_height));
}

TEST(test_particle, x_past_right_limit_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, 2.0, 98.1, 25.0), box_width, box_height));
}

TEST(test_particle, y_exactly_on_both_limits_is_valid){
    EXPECT_TRUE(aos::is_valid_particle(make_particle(10.0, 2.0, 50.0, 2.0), box_width, box_height));
    EXPECT_TRUE(aos::is_valid_particle(make_particle(10.0, 2.0, 50.0, 48.0), box_width, box_height));
}

TEST(test_particle, y_outside_limits_is_invalid){
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, 2.0, 50.0, 1.9), box_width, box_height));
    EXPECT_FALSE(aos::is_valid_particle(make_particle(10.0, 2.0, 50.0, 48.1), box_width, box_height));
}