#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "config_writer.hpp"

namespace {

// Each test writes into its own file in the system temp folder, and removes it afterwards.
class ConfigWriterTest : public ::testing::Test {
protected:
  std::filesystem::path path_ = std::filesystem::temp_directory_path() / "grav_test_config.txt";

  void TearDown() override { std::filesystem::remove(path_); }

  // Returns the whole content of the file as a string.
  [[nodiscard]] std::string read_all() const {
    std::ifstream in{path_};
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
  }
};

}  // namespace

// The example of the spec: header (width, height, time step, count) and one line per particle.
TEST_F(ConfigWriterTest, WritesHeaderAndParticles) {
  std::vector<common::particle_record> const particles{
      {125.72, 2.54, common::point{10.5, 12.25}, common::vec2d{43.225, 75.325}},
      {200.3, 15.0, common::point{50.0, 60.0}, common::vec2d{5.0, 10.0}}};

  EXPECT_TRUE(common::write_config(path_, particles, 1000.0, 2000.0, 0.25));

  EXPECT_EQ(read_all(),
            "1000.000 2000.000 0.250 2\n"
            "125.720 2.540 10.500 12.250 43.225 75.325\n"
            "200.300 15.000 50.000 60.000 5.000 10.000\n");
}