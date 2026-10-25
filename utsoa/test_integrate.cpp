#include "particle_system.hpp"
#include <gtest/gtest.h>

namespace{
    constexpr double box_width = 100.0;
    constexpr double box_height = 50.0;

    // Build the values of one particle with radius 2 and mass 1 at (x,y) moving at (vx,vy)
    soa::particle_values moving_particle (double x, double y, double vx, double vy) {
        soa::particle_values p;
        p.mass = 1.0;  // any postitive value - doesnt matter for walls 
        p.radius = 2.0;
        p.x = x;
        p.y = y;
        p.vx = vx;
        p.vy = vy;
        return p;
    }
}

// --- bounce_off_walls testing ---
TEST(test_integrate, rebound_inside_box_changes_nothing){
    soa::particle_system system;  // empty space
    soa::add_particle(system, moving_particle(50.0, 25.0, -3.0, 4.0)); // in the middle
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 50.0);  
    EXPECT_EQ(system.y.front(), 25.0);
    EXPECT_EQ(system.vx.front(), -3.0);
    EXPECT_EQ(system.vy.front(), 4.0);
}

TEST(test_integrate, rebound_left_wall){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(1.0, 25.0, -3.0, 0.0)); // left edge at 1-2=-1 past wall moving left
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 2.0);  // back on wall x = radius
    EXPECT_EQ(system.vx.front(), 3.0); // now moving right, into box
}

TEST(test_integrate, rebound_when_exactly_touching_left_wall){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(2.0, 25.0, -3.0, 0.0)); // left edge at 0
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 2.0);  // stay on wall
    EXPECT_EQ(system.vx.front(), 3.0); // turn around
}

TEST(test_integrate, rebound_keeps_inward_velocity_inward){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(1.0, 25.0, 3.0, 0.0)); // past left well but already mvoing right
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.vx.front(), 3.0); // must stay positive - flipping sign would send it back out
}

TEST(test_integrate, rebound_right_wall){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(99.0, 25.0, 3.0, 0.0)); // right edge at 99+2=101 past wall
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 98.0);  // width - radius = 100-2
    EXPECT_EQ(system.vx.front(), -3.0); // now moving left, into box
}

TEST(test_integrate, rebound_bottom_wall){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(50.0, 1.0, 0.0, -4.0)); // bottom edge at -1, moving down
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.y.front(), 2.0);  // back on the bottom wall
    EXPECT_EQ(system.vy.front(), 4.0); // now moving up
}

TEST(test_integrate, rebound_top_wall){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(50.0, 49.0, 0.0, 4.0)); // top edge at 51, moving up
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.y.front(), 48.0);  // height - radius = 50-2
    EXPECT_EQ(system.vy.front(), -4.0); // now moving down
}

TEST(test_integrate, rebound_corner_fixes_both_directions){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(1.0, 49.0, -3.0, 4.0)); // past the left and top wall at once
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 2.0);  // x fixed
    EXPECT_EQ(system.y.front(), 48.0); // y fixed
    EXPECT_EQ(system.vx.front(), 3.0);  // x vel inward
    EXPECT_EQ(system.vy.front(), -4.0); // y vel turned inward
}

TEST(test_integrate, rebound_handles_every_particle_separately){
    soa::particle_system system;  
    soa::add_particle(system, moving_particle(1.0, 25.0, -3.0, 0.0)); // particle 0: past the left wall
    soa::add_particle(system, moving_particle(50.0, 25.0, -3.0, 0.0)); // particle 1: in the middle, same velocity
    soa::bounce_off_walls(system, box_width, box_height);
    EXPECT_EQ(system.x.front(), 2.0);  // particle 0 was not put back on the wall
    EXPECT_EQ(system.vx.front(), 3.0);  // turn around
    EXPECT_EQ(system.x.back(), 50.0);  // particle 1 was left alone
    EXPECT_EQ(system.vx.back(), -3.0); // still moving left
}

