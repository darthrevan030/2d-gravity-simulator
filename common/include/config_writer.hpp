#ifndef COMMON_CONFIG_WRITER_HPP
#define COMMON_CONFIG_WRITER_HPP

#include <filesystem>
#include <vector>

#include "particle_record.hpp"

namespace common {

[[nodiscard]] bool write_config(std::filesystem::path const & output_path,
                                std::vector<particle_record> const & particles);

} 

#endif  
