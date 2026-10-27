#include <ostream>
#include <format>
#include <vector>
#include <fstream>
#include "config_writer.hpp"
namespace common{

namespace{
void write_particle(std::ostream & out, particle_record const & p){
    out << std::format("{:.3f} {:.3f} {:.3f} {:.3f} {:.3f} {:.3f}\n", p.mass, p.radius, p.pos.getx(), p.pos.gety(), p.vel.getx(), p.vel.gety()  );

}
void write_header(std::ostream & out, enclosure box, double timestep, size_t particle_number){
    out << std::format("{:.3f} {:.3f} {:.3f} {}\n", box.width, box.height, timestep, particle_number);
 }}
bool write_config(std::filesystem::path const & output_path,
                                std::vector<particle_record> const & particles, enclosure box, double timestep){
    std::ofstream out{output_path};

    if (!out) { return false; }

        write_header(out, box,timestep,particles.size());

 for (auto const & particle : particles) {
  write_particle(out, particle);
}
return true;


} }


