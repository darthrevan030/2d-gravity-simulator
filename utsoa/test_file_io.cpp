#include "particle_system.hpp" 

#include <gtest/gtest.h> 

#include <cstddef>  
#include <sstream>  // std::istringstream / std::ostringstream: text in memory instead of real files
#include <vector>   

// ---- read_particles ----
TEST(test_file_io, reads_valid_lines) {
    std::istringstream input{"10.0 2.0 30.0 40.0 5.0 -6.0\n1.0 1.0 50.0 25.0 0.0 0.0\n"}; // two lines, \n = new line
    soa::particle_system const system = soa::read_particles(input, 100.0, 50.0); // box 100 x 50
    ASSERT_EQ(soa::particle_count(system), 2U);
    EXPECT_EQ(system.mass.front(), 10.0); // first line's values
    EXPECT_EQ(system.vy.front(), -6.0);
    EXPECT_EQ(system.x.back(), 50.0); // second line's values
}

TEST(test_file_io, empty_input_gives_no_particles) {
    std::istringstream input{""};  // nothing at all
    EXPECT_EQ(soa::particle_count(soa::read_particles(input, 100.0, 50.0)), 0U);
}

TEST(test_file_io, skips_lines_with_too_few_or_too_many_numbers) {
    std::istringstream input{"1.0 1.0 10.0 10.0 0.0\n" // 5 numbers: skipped
                             "1.0 1.0 10.0 10.0 0.0 0.0 7.0\n" // 7 numbers: skipped
                             "1.0 1.0 20.0 20.0 0.0 0.0\n"}; // valid
    soa::particle_system const system = soa::read_particles(input, 100.0, 50.0);
    ASSERT_EQ(soa::particle_count(system), 1U);
    EXPECT_EQ(system.x.front(), 20.0);  // only the valid line was read
}

TEST(test_file_io, skips_lines_with_words_or_infinity) {
    std::istringstream input{"one 1.0 10.0 10.0 0.0 0.0\n" // a word instead of a number
                             "1.0 1.0 10.0 10.0 inf 0.0\n"  // infinity
                             "1.0 1.0 20.0 20.0 0.0 0.0\n"}; // valid
    EXPECT_EQ(soa::particle_count(soa::read_particles(input, 100.0, 50.0)), 1U);
}

TEST(test_file_io, skips_incomplete_last_line_without_newline) {
    std::istringstream input{"1.0 1.0 20.0 20.0 0.0 0.0\n1.0 1.0 30.0"};  // last line cut off
    EXPECT_EQ(soa::particle_count(soa::read_particles(input, 100.0, 50.0)), 1U);
}

TEST(test_file_io, invalid_particles_are_discarded_but_use_up_their_id) {
    std::istringstream input{"1.0 1.0 10.0 10.0 0.0 0.0\n" // valid -> id 0
                             "0.0 1.0 20.0 20.0 0.0 0.0\n" // id 1, mass 0: discarded
                             "1.0 1.0 99.5 20.0 0.0 0.0\n" // id 2, sticks out of the box: discarded
                             "2.0 1.0 30.0 30.0 0.0 0.0\n"}; // valid -> id 3
    soa::particle_system const system = soa::read_particles(input, 100.0, 50.0);
    std::vector<std::size_t> const expected_ids{0, 3};
    EXPECT_EQ(system.id, expected_ids);
    EXPECT_EQ(system.mass.back(), 2.0);
}

TEST(test_file_io, malformed_lines_also_up_an_id) {
    std::istringstream input{"1.0 1.0 10.0 10.0 0.0 0.0\n" // id 0
                             "hello world\n"  // id 1: malformed, discarded
                             "\n" // id 2: empty line, discarded
                             "2.0 1.0 30.0 30.0 0.0 0.0\n"}; // id 3
    soa::particle_system const system = soa::read_particles(input, 100.0, 50.0);
    std::vector<std::size_t> const expected_ids{0, 3}; // ids 1 and 2 were used by the discarded lines
    EXPECT_EQ(system.id, expected_ids);
}

// ---- write_particles ----
TEST(test_file_io, writes_header_and_lines_with_three_decimals) {
    soa::particle_system system;
    soa::particle_values p;
    p.mass   = 10.0;
    p.radius = 2.5;
    p.x      = 30.12345; // more decimals than we print
    p.y      = 40.0;
    p.vx     = -1.0;
    p.vy     = 0.5;
    soa::add_particle(system, p);
    soa::simulation_settings settings;
    settings.width     = 1000.0;
    settings.height    = 2000.0;
    settings.time_step = 0.25;

    std::ostringstream output; // collects the text in memory
    soa::write_particles(output, system, settings);

    EXPECT_EQ(output.str(), "1000.000 2000.000 0.250 1\n" // exact expected text
                            "0: 10.000 2.500 30.123 40.000 -1.000 0.500\n");
}

TEST(test_file_io, writes_only_the_header_for_an_empty_system) {
    soa::particle_system const system;
    soa::simulation_settings settings;
    settings.width     = 100.0;
    settings.height    = 50.0;
    settings.time_step = 1.0;
    std::ostringstream output;
    soa::write_particles(output, system, settings);
    EXPECT_EQ(output.str(), "100.000 50.000 1.000 0\n"); // header with count 0, no particle lines
}
