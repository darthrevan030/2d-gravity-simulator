#ifndef COMMON_INPUT_PARSER_HPP
#define COMMON_INPUT_PARSER_HPP

#include "particle_record.hpp"
#include <istream>
#include <vector>
#include "enclosure.hpp"
namespace common {

[[nodiscard]] std::vector<particle_record> parse_input_stream(std::istream & input, enclosure const & box);

}  // namespace common

#endif

