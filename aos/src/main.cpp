#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <config_writer.hpp>
#include <enclosure.hpp>
#include <input_parser.hpp>
#include <progargs.hpp>

#include "particle_system.hpp"

int main(int argc, char ** argv) try {
  std::span const spnargs{argv, static_cast<std::size_t>(argc)};
  std::vector<std::string> const vecargs{spnargs.begin(), spnargs.end()};
  common::program_arguments const progargs{vecargs};

  common::enclosure const box{progargs.width(), progargs.height()};

  std::ifstream input{"input/input.txt"};
  if (!input) {
    throw std::runtime_error{"cannot open input/input.txt"};
  }
  auto particles = common::parse_input_stream(input, box);
  aos::particle_system system{std::move(particles)};

  if (!common::write_config("init_config.txt", system.particles(), box, progargs.time_step())) {
    throw std::runtime_error{"cannot write init_config.txt"};
  }
  system.simulate(progargs.num_iterations(), progargs.time_step(), progargs.width(),
                  progargs.height());
  if (!common::write_config("final_config.txt", system.particles(), box, progargs.time_step())) {
    throw std::runtime_error{"cannot write final_config.txt"};
  }

  return EXIT_SUCCESS;

} catch (std::exception const & e) {
  std::cerr << "grav-aos: " << e.what() << '\n';
  return EXIT_FAILURE;

} catch (...) {
  std::cerr << "grav-aos: Unknown exception" << '\n';
  return EXIT_FAILURE;
}
