#include "progargs.hpp"

#include <gtest/gtest.h>
namespace {
std::string message_of(std::vector<std::string> const & args) {
  try {
    common::program_arguments const parsed{args};
  } catch (std::invalid_argument const & e) {
    return e.what();
  }
  return "no exception";
}
}  // namespace

TEST(test_progargs, zero_arguments) {
  std::vector<std::string> const args {};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, no_arguments) {
  std::vector<std::string> const args{"program_name"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_zero) {
  std::vector<std::string> const args{"program_name", "0", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_negative) {
  std::vector<std::string> const args{"program_name", "-10", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_non_numeric) {
  std::vector<std::string> const args{"program_name", "abc", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10abc", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, iterations_cantconvert_message) {
  EXPECT_EQ(message_of({"program_name", "abs", "-1000", "2000.0", "0.25"}),
            "Invalid value for 'num_iterations': 'abs'");
}
TEST(test_progargs, iterations_negative_message) {
  EXPECT_EQ(message_of({"program_name", "-10", "1000", "2000.0", "0.25"}),
            "Invalid value for num_iterations: -10");
}
TEST(test_progargs, iterations_not_integer_message) {
  EXPECT_EQ(message_of({"program_name", "10.5", "1000", "2000.0", "0.25"}),
            "Invalid value for 'num_iterations': '10.5'");
}


TEST(test_progargs, invalid_width_zero) {
  std::vector<std::string> const args{"program_name", "10", "0.0", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_negative) {
  std::vector<std::string> const args{"program_name", "10", "-1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "abc", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5abc", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_infinity) {
  std::vector<std::string> const args{"program_name", "10", "inf", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_nan) {
  std::vector<std::string> const args{"program_name", "10", "nan", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, width_cantconvert_message) {
  EXPECT_EQ(message_of({"program_name", "10", "abs", "2000.0", "0.25"}),
            "Invalid value for 'width': 'abs'");
}
TEST(test_progargs, width_negative_message) {
  EXPECT_EQ(message_of({"program_name", "10", "-1000.5", "2000.0", "0.25"}),
            "Invalid value for width: -1000.5");
}


TEST(test_progargs, invalid_height_zero) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "0.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_negative) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "-2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "abc", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0abc", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_infinity) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "inf", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_nan) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "nan", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, height_cantconvert_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "abs", "0.25"}),
            "Invalid value for 'height': 'abs'");
}
TEST(test_progargs, height_negative_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "-2000.0", "0.25"}),
            "Invalid value for height: -2000");
}
TEST(test_progargs, invalid_timestep_zero) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.0"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_negative) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "-0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "abc"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, invalid_timestep_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.25abc"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_infinity) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "inf"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_nan) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "nan"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, timestep_cantconvert_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "2000.0", "abs"}),
            "Invalid value for 'time_step': 'abs'");
}
TEST(test_progargs, timestep_negative_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "2000.0", "-0.25"}),
            "Invalid value for time_step: -0.25");
}

TEST(test_progargs, too_few_arguments) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, too_few_arguments_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "2000.0"}),
            "program_name: invoked with 3 arguments\n"
            "\tExpected 4 arguments: <num_iterations> <width> <height> <time_step>\n"
            "\tReceived arguments: [\"10\", \"1000.5\", \"2000.0\"]");
}
TEST(test_progargs, too_many_arguments) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.25", "extra_argument"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}
TEST(test_progargs, too_many_arguments_message) {
  EXPECT_EQ(message_of({"program_name", "10", "1000.5", "2000.0", "0.25", "extra_argument"}),
            "program_name: invoked with 5 arguments\n"
            "\tExpected 4 arguments: <num_iterations> <width> <height> <time_step>\n"
            "\tReceived arguments: [\"10\", \"1000.5\", \"2000.0\", \"0.25\", \"extra_argument\"]");
}

TEST(test_progargs, valid_arguments) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.25"};
  common::program_arguments const parsed_args{args};
  EXPECT_EQ(parsed_args.program_name(), "program_name");
  EXPECT_EQ(parsed_args.num_iterations(), 10);
  EXPECT_DOUBLE_EQ(parsed_args.width(), 1000.5);
  EXPECT_DOUBLE_EQ(parsed_args.height(), 2000.0);
  EXPECT_DOUBLE_EQ(parsed_args.time_step(), 0.25);
}