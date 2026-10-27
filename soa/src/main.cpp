#include "particle_system.hpp" // everything in soa::

#include <progargs.hpp> // professor's argument checker (common::program_arguments)

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <exception> // std::exception
#include <fstream> // std::ifstream (read a file), std::ofstream (write a file)
#include <iostream> // std::cerr (error output)
#include <span> // std::span
#include <stdexcept> // std::runtime_error
#include <string> // std::string
#include <vector> // std::vector

int main(int argc, char ** argv) try { // try: any error jumps to catch below
  std::span const spnargs{argv, static_cast<std::size_t>(argc)}; // professor's code: wrap argv safely
  std::vector<std::string> const vecargs{spnargs.begin(), spnargs.end()}; // turn the arguments into strings
  common::program_arguments const progargs{vecargs}; // check all 4 (throws if anything is wrong)

  soa::simulation_settings settings; // copy the 4 values into own struct
  settings.iterations = progargs.num_iterations();
  settings.width      = progargs.width();
  settings.height     = progargs.height();
  settings.time_step  = progargs.time_step();

  std::ifstream input_file{"input/input.txt"};  // open the input, relative to where the program is run
  if (not input_file) { throw std::runtime_error{"cannot open input/input.txt"}; }  // missing file -> error
  soa::particle_system system = soa::read_particles(input_file, settings.width, settings.height);

  soa::resolve_collisions(system); // collisions ONCE before the simulation starts
  std::ofstream init_file{"init_config.txt"}; // create (or overwrite) the first output file
  soa::write_particles(init_file, system, settings);

  soa::run_simulation(system, settings); // all the iterations
  std::ofstream final_file{"final_config.txt"}; // create (or overwrite) the second output file
  soa::write_particles(final_file, system, settings);
  return EXIT_SUCCESS; // 0 = everything went fine

} catch (std::exception const & e) { // any error: message on stderr, prefixed with the program name
  std::cerr << "grav-soa: " << e.what() << '\n';
  return EXIT_FAILURE; // non-zero = something went wrong

} catch (...) { // anything that isn't a std::exception (shouldn't happen)
  std::cerr << "grav-soa: Unknown exception" << '\n';
  return EXIT_FAILURE;
}