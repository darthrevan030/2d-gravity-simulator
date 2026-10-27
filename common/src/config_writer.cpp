#include <ostream>
#include <format>
#include <utility>
#include <vector>
#include <fstream>
#include "config_writer.hpp"
namespace common{
namespace{
void write_particle(std::ostream & out, particle_record const & p){
    out << std::format("{:.3f} {:.3f} {:.3f} {:.3f} {:.3f} {:.3f}\n", p.mass, p.radius, p.pos.getx(), p.pos.gety(), p.vel.getx(), p.vel.gety()  );

}
void write_header(std::ostream & out,double width, double height, double timestep, size_t particle_number){
    out << std::format("{:.3f} {:.3f} {:.3f} {}\n", width, height, timestep, particle_number);
 }}
bool write_config(std::filesystem::path const & output_path,
                                std::vector<particle_record> const & particles, double width, double height, double timestep){
    std::ofstream out{output_path};

    if (!out) { return false; }

    write_header(out, width, height,timestep,particles.size());

    for (std::size_t i=0;i<particles.size();i++){

        write_particle(out,particles[i]);

    }
    return true;


} }


