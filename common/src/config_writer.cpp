#include "config_writer.hpp"

namespace common {

bool write_config(std::filesystem::path const & output_path,
                 std::vector<particle_record> const & particles) {
  (void)output_path;
  (void)particles;
  return false;
}

}
