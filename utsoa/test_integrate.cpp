#include "particle_system.hpp"
#include <gtest/gtest.h>
#include <gsl/gsl> // gsl::at

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
    EXPECT_EQ(system.vx.front(), 3.0); // turn around
    EXPECT_EQ(system.x.back(), 50.0); // particle 1 was left alone
    EXPECT_EQ(system.vx.back(), -3.0); // still moving left
}

// --- move_particles testing ---
TEST(test_integrate, move_without_acceleration_goes_in_a_straight_line) {
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 20.0, 2.0, -3.0)); // at (10,20), velocity(2,-3), acc 0
    soa::move_particles(system, 0.5); // one step with dt = 0.5
    EXPECT_EQ(system.vx.front(), 2.0); // no acceleration: velocity unchanged
    EXPECT_EQ(system.vy.front(), -3.0);
    EXPECT_EQ(system.x.front(), 11.0); // 10 + 2 * 0.5
    EXPECT_EQ(system.y.front(), 18.5); // 20 + (-3) * 0.5
}

TEST(test_integrate, move_applies_acceleration_to_velocity) {
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 20.0, 1.0, 1.0)); // velocity (1, 1)
    system.ax.front() = 4.0; // set the acceleration by hand (gravity comes in Step 5)
    system.ay.front() = -2.0;
    soa::move_particles(system, 0.5);
    EXPECT_EQ(system.vx.front(), 3.0); // 1 + 4 * 0.5
    EXPECT_EQ(system.vy.front(), 0.0); // 1 + (-2) * 0.5
}

TEST(test_integrate, move_uses_the_new_velocity_for_position) {
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 20.0, 1.0, 1.0));  // same setup as the previous test
    system.ax.front() = 4.0;
    system.ay.front() = -2.0;
    soa::move_particles(system, 0.5); // new velocity is (3, 0), as computed above
    EXPECT_EQ(system.x.front(), 11.5); // 10 + 3 * 0.5  (NEW vx = 3; the old vx = 1 would give 10.5)
    EXPECT_EQ(system.y.front(), 20.0); // 20 + 0 * 0.5  (NEW vy = 0; the old vy = 1 would give 20.5)
}


TEST(test_integrate, move_does_not_change_acceleration) {
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 20.0, 1.0, 1.0));
    system.ax.front() = 4.0;
    system.ay.front() = -2.0;
    soa::move_particles(system, 0.5);
    EXPECT_EQ(system.ax.front(), 4.0); // gives the box vector to read and to write to it
    EXPECT_EQ(system.ay.front(), -2.0);
}

TEST(test_integrate, move_handles_every_particle_separately) {
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 20.0, 2.0, 0.0)); // particle 0: moving right at 2
    soa::add_particle(system, moving_particle(50.0, 30.0, -4.0, 0.0)); // particle 1: moving left at 4
    soa::move_particles(system, 0.5);
    EXPECT_EQ(system.x.front(), 11.0); // particle 0: 10 + 2 * 0.5
    EXPECT_EQ(system.x.back(), 48.0); // particle 1: 50 + (-4) * 0.5
}

// --- compute_accelerations testing ---

namespace {

    /// Values of a particle at rest with the given mass at (x, y), radius 1.
    soa::particle_values resting_particle(double mass, double x, double y) {
        soa::particle_values p;
        p.mass   = mass;
        p.radius = 1.0;
        p.x      = x;
        p.y      = y;
        return p;
    }

    constexpr double tolerance = 1e-12;  // how close two decimals must be to count as equal

}  // namespace

TEST(test_integrate, gravity_two_bodies_match_hand_calculation) {
    // distance 5 (3-4-5 triangle), G * m * m = 6.674e-11 * 1e20 = 6.674e9
    // F = 6.674e9 * (3, 4) / 125 = (1.60176e8, 2.13568e8); a = F / 1e10 = (0.0160176, 0.0213568)
    soa::particle_system system;
    soa::add_particle(system, resting_particle(1e10, 10.0, 10.0));
    soa::add_particle(system, resting_particle(1e10, 13.0, 14.0));
    soa::compute_accelerations(system);
    EXPECT_NEAR(system.ax.front(), 0.0160176, tolerance); // pulled toward the other one
    EXPECT_NEAR(system.ay.front(), 0.0213568, tolerance);
    EXPECT_NEAR(system.ax.back(), -0.0160176, tolerance); // pulled the opposite way
    EXPECT_NEAR(system.ay.back(), -0.0213568, tolerance);
}

TEST(test_integrate, gravity_forces_are_equal_and_opposite) {
    soa::particle_system system;
    soa::add_particle(system, resting_particle(1e10, 10.0, 10.0));
    soa::add_particle(system, resting_particle(3e10, 13.0, 14.0)); // three times heavier
    soa::compute_accelerations(system);
    // F = m * a must be the same size on both, opposite direction
    EXPECT_NEAR(system.ax.front() * 1e10, -system.ax.back() * 3e10, 1e-3);
    EXPECT_NEAR(system.ay.front() * 1e10, -system.ay.back() * 3e10, 1e-3);
}

TEST(test_integrate, gravity_middle_of_symmetric_line_feels_nothing) {
    soa::particle_system system;
    soa::add_particle(system, resting_particle(1e10, 10.0, 10.0));
    soa::add_particle(system, resting_particle(1e10, 20.0, 10.0)); // exactly in the middle
    soa::add_particle(system, resting_particle(1e10, 30.0, 10.0));
    soa::compute_accelerations(system);
    EXPECT_EQ(gsl::at(system.ax, 1), 0.0); // pulled equally left and right
    EXPECT_EQ(gsl::at(system.ay, 1), 0.0);
}

TEST(test_integrate, gravity_single_particle_has_no_acceleration) {
    soa::particle_system system;
    soa::add_particle(system, resting_particle(5.0, 10.0, 10.0));
    soa::compute_accelerations(system);
    EXPECT_EQ(system.ax.front(), 0.0);
    EXPECT_EQ(system.ay.front(), 0.0);
}

TEST(test_integrate, gravity_is_recomputed_not_added_up) {
    soa::particle_system system;
    soa::add_particle(system, resting_particle(1e10, 10.0, 10.0));
    soa::add_particle(system, resting_particle(1e10, 13.0, 14.0));
    soa::compute_accelerations(system);
    soa::compute_accelerations(system); // second call must not add on top of the first
    EXPECT_NEAR(system.ax.front(), 0.0160176, tolerance);
}


// --- simulate_step and run_simulation testing --

TEST(test_integrate, step_does_gravity_then_movement) {
    soa::simulation_settings settings;   // box 100 x 100, dt = 1
    settings.width     = 100.0;
    settings.height    = 100.0;
    settings.time_step = 1.0;
    soa::particle_system system;
    soa::add_particle(system, resting_particle(1e10, 10.0, 10.0)); // same pair as the gravity test
    soa::add_particle(system, resting_particle(1e10, 13.0, 14.0));
    soa::simulate_step(system, settings);
    EXPECT_NEAR(system.vx.front(), 0.0160176, tolerance);        // v = 0 + a * 1 (gravity happened)
    EXPECT_NEAR(system.x.front(), 10.0 + 0.0160176, tolerance);  // moved with the NEW velocity
}

TEST(test_integrate, run_repeats_the_step) {
    soa::simulation_settings settings;
    settings.iterations = 3;             // three steps
    settings.width      = 100.0;
    settings.height     = 100.0;
    settings.time_step  = 1.0;
    soa::particle_system system;
    soa::add_particle(system, moving_particle(10.0, 25.0, 2.0, 0.0)); // alone: no gravity, far from walls
    soa::run_simulation(system, settings);
    EXPECT_EQ(system.x.front(), 16.0);   // 10 + 3 steps * 2
}
