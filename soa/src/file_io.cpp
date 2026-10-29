#include "particle_system.hpp"

#include <cmath> // std::isfinite - checking if normal number
#include <cstddef> // std::size_t
#include <istream> // std::istream, std::getline
#include <optional> // std::optional ("a value, or nothing")
#include <ostream> // std::ostream
#include <print> // std::println (formatted output)
#include <ranges> // std::views::zip
#include <sstream> // std::istringstream (read numbers out of one line)
#include <string> // std::string

namespace soa{
    namespace { 
        // reads exactly 6 numbers from one line, otherwise line doesn't conform and does nothing
        std::optional<particle_values> parse_line(std::string const & line) {
            std::istringstream in{line}; // treat the line like a little input stream
            particle_values p; // all zeros to start
            if (not(in >> p.mass >> p.radius >> p.x >> p.y >> p.vx >> p.vy)) { return std::nullopt; } // fewer than 6 numbers
            std::string extra;
            if (in >> extra) { return std::nullopt; } // something left over: a 7th value or text
            bool const all_finite = std::isfinite(p.mass) and std::isfinite(p.radius) and
                                    std::isfinite(p.x) and std::isfinite(p.y) and std::isfinite(p.vx) and
                                    std::isfinite(p.vy);
            if (not all_finite) { return std::nullopt; } // inf or nan: not a usable particle
            return p; // six good numbers
        }
    } // namespace



    particle_system read_particles(std::istream & input, double width, double height) {
        particle_system system; // starts empty
        std::size_t next_id = 0; // id for the next line that is read
        std::string line;
        while (std::getline(input, line)) { // one line at a time until the end of the input
            std::size_t const line_id = next_id; // line gets its id as it is read
            ++next_id; // every line read uses an id even if it is thrown away below
            std::optional<particle_values> const values = parse_line(line);
            if (not values) { continue; } // malformed line: discarded, its id is just lost
            if (is_valid_particle(*values, width, height)) {
                add_particle(system, *values);
                system.id.back() = line_id; // keep this line's id (gaps where lines were thrown away)
            }
        }
        return system;
    }



    void write_particles(std::ostream & output, particle_system const & system, simulation_settings const & settings) {
        std::println(output, "{:.3f} {:.3f} {:.3f} {}", settings.width, settings.height,
                     settings.time_step, particle_count(system)); // header: width height dt count
        for (auto && [id, m, r, x, y, vx, vy] :
             std::views::zip(system.id, system.mass, system.radius, system.x, system.y, system.vx, system.vy)) {
            std::println(output, "{}: {:.3f} {:.3f} {:.3f} {:.3f} {:.3f} {:.3f}", id, m, r, x, y, vx, vy); // one particle
        }
    }
}