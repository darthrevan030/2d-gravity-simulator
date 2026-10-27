#include "particle_system.hpp"
#include <gtest/gtest.h>
#include <gsl/gsl>
#include <cstddef>
#include <iterator>
#include <random>
#include <vector>

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

    EXPECT_EQ(soa::particle_count(system), 2U); // still 2: removed later when we remove from all 9 vectors at once
    EXPECT_EQ(system.vy.size(), 2U); // vectors are all still the same length
}


// ---- remove_abdorbed and resolve_collisions ---

namespace{
    // slow reference version statment: find the 1st overlapping pair in id order, merge it, remove the loser, start again from beginning
    // return false when no pair is left
    bool merge_first_collision(soa::particle_system & system) {
        gsl::index const n = std::ssize(system.mass);
        for (gsl::index i = 0; i < n; ++i) {
            for (gsl::index j = i + 1; j < n; ++j) {
                if (not soa::particles_collide(system, i, j)) { continue; }
                if (soa::first_absorbs_second(system, i, j)) {
                    soa::absorb(system, i, j);
                    gsl::at(system.mass, j) = 0.0;
                } else {
                    soa::absorb(system, j, i);
                    gsl::at(system.mass, i) = 0.0;
                }
                soa::remove_absorbed(system);  // remove immediately...
                return true;                   // ...and restart from the beginning
            }
        }
        return false;
    }
    // keeps merging until no overlpaping pair is left, word for word to ensure correctness
    void resolve_collisions_slowly(soa::particle_system & system) {
        while (merge_first_collision(system)) {} // real system passed in, system merges when done
    }

    // many random particles packed in so many collisions
    // masses are only 1, 2 or 3, so ties (and the tie-break rules) happen often
    soa::particle_system random_crowded_system(unsigned int seed, int count) {
        std::mt19937 generator{seed}; // fixed seed: the same "random" particles every run
        std::uniform_real_distribution<double> position{5.0, 95.0};
        std::uniform_real_distribution<double> radius{0.5, 3.0};
        std::uniform_real_distribution<double> velocity{-1.0, 1.0};
        std::uniform_int_distribution<int> mass{1, 3};
        soa::particle_system system;
        for (int k = 0; k < count; ++k) {
            soa::particle_values p;
            p.mass   = static_cast<double>(mass(generator));
            p.radius = radius(generator);
            p.x      = position(generator);
            p.y      = position(generator);
            p.vx     = velocity(generator);
            p.vy     = velocity(generator);
            soa::add_particle(system, p);
        }
        return system;
    }

} // namespace

TEST(test_collisions, remove_absorbed_keeps_order_and_ids) {
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 1.0, 10.0, 10.0)); // id 0
    soa::add_particle(system, make_values(2.0, 1.0, 20.0, 20.0)); // id 1
    soa::add_particle(system, make_values(3.0, 1.0, 30.0, 30.0)); // id 2
    gsl::at(system.mass, 1) = 0.0;                                // mark id 1 as absorbed

    soa::remove_absorbed(system);

    std::vector<std::size_t> const expected_ids{0, 2};
    EXPECT_EQ(system.id, expected_ids);  // id 1 is gone, the others keep their order
    EXPECT_EQ(system.x.back(), 30.0);    // id 2's values moved down with it
    EXPECT_EQ(system.ay.size(), 2U);     // every vector shrank
    EXPECT_EQ(system.radius.size(), 2U);
}

TEST(test_collisions, resolve_does_nothing_without_collisions) {
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 2.0, 10.0, 10.0));
    soa::add_particle(system, make_values(1.0, 2.0, 14.0, 10.0)); // exactly touching: no collision
    soa::resolve_collisions(system);
    EXPECT_EQ(soa::particle_count(system), 2U);
}

TEST(test_collisions, resolve_handles_empty_system) {
    soa::particle_system system;
    soa::resolve_collisions(system); // must not crash
    EXPECT_EQ(soa::particle_count(system), 0U);
}

TEST(test_collisions, resolve_merges_a_chain_in_id_order) {
    // same input as the team's merge_chain functional test
    soa::particle_system system;
    soa::particle_values p0 = make_values(5.0, 1.0, 50.0, 50.0);
    p0.vx = 1.0;
    soa::particle_values p1 = make_values(10.0, 1.5, 52.0, 50.0);
    p1.vy = 1.0;
    soa::particle_values p2 = make_values(3.0, 1.0, 54.0, 50.0);
    p2.vx = 2.0;
    p2.vy = 2.0;
    soa::add_particle(system, p0);                                // id 0
    soa::add_particle(system, p1);                                // id 1
    soa::add_particle(system, p2);                                // id 2
    soa::add_particle(system, make_values(7.0, 1.0, 80.0, 80.0)); // id 3: far away

    soa::resolve_collisions(system);

    std::vector<std::size_t> const expected_ids{1, 3};
    EXPECT_EQ(system.id, expected_ids);
    EXPECT_EQ(system.mass.front(), 18.0); // 5 + 10 + 3
    EXPECT_EQ(system.x.front(), 52.0);    // id 1 kept its own position
    EXPECT_EQ(system.vx.front(), 3.0);    // 1 + 0 + 2
    EXPECT_EQ(system.vy.front(), 3.0);    // 0 + 1 + 2
}

TEST(test_collisions, resolve_continues_after_the_first_particle_is_absorbed) {
    // id 0 is absorbed by id 1; the scan must still check id 1 against id 2
    soa::particle_system system;
    soa::add_particle(system, make_values(1.0, 1.0, 10.0, 10.0)); // id 0: light
    soa::add_particle(system, make_values(5.0, 1.0, 11.0, 10.0)); // id 1: heavy, overlaps id 0
    soa::add_particle(system, make_values(1.0, 1.0, 12.5, 10.0)); // id 2: overlaps id 1 only
    soa::resolve_collisions(system);
    ASSERT_EQ(soa::particle_count(system), 1U);
    EXPECT_EQ(system.id.front(), 1U);
    EXPECT_EQ(system.mass.front(), 7.0);  // 1 + 5 + 1
}

TEST(test_collisions, fast_scan_matches_slow_restart_scan) {
    for (unsigned int seed = 1; seed <= 5; ++seed) {      // five different random inputs
        soa::particle_system fast = random_crowded_system(seed, 200);
        soa::particle_system slow = fast;                   // identical copy

        soa::resolve_collisions(fast);                      // our fast scan
        resolve_collisions_slowly(slow);                    // by-the-book restart scan

        EXPECT_LT(soa::particle_count(fast), 200U);         // collisions really happened
        EXPECT_EQ(fast.id, slow.id);                        // same survivors...
        EXPECT_EQ(fast.mass, slow.mass);                    // ...with exactly the same values
        EXPECT_EQ(fast.vx, slow.vx);
        EXPECT_EQ(fast.vy, slow.vy);
        EXPECT_EQ(fast.x, slow.x);
    }
}