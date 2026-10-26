#include <cstdlib>
#include <exception>
#include <iostream>
#include <print>
#include <span>
#include <string>
#include <vector>

#include <progargs.hpp>

int main(int argc, char ** argv) try {
  std::span const spnargs{argv, static_cast<std::size_t>(argc)};
  std::vector<std::string> const vecargs{spnargs.begin(), spnargs.end()};
  common::program_arguments const progargs{vecargs};

  std::println("grav-aos:");
  std::println("\tnum_iterations: {}\n\twidth: {}\n\theight: {}\n\ttime_step: {}",
               progargs.num_iterations(), progargs.width(), progargs.height(),
               progargs.time_step());

  // Rest of program
  std::ifstream input{"input/input.txt"};
  if(!input){
    throw std::runtime_error{"Couldn't open file"};
  }
  auto particles = common::parse_input_stream(input, progars.width(), progars.height());
  aos::particle_system system{std::move(particles)};

  common::write_config("init_config.txt", system.particles(), progargs.width(), progargs.height(), 0);
  system.simulate(progargs.num_iterations(), progargs.time_step(), progargs.width(), progargs.height());
  common::write_config("final_config.txt", system.particles(), progargs.width(), progargs.height(), progargs.num_iterations());

  return EXIT_SUCCESS;

} catch (std::exception const & e) {
  std::cerr << "grav-aos: " << e.what() << '\n';
  return EXIT_FAILURE;

} catch (...) {
  std::cerr << "grav-aos: Unknown exception" << '\n';
  return EXIT_FAILURE;
}
