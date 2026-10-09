#ifndef COMMON_INPUT_PARSER_HPP
#define COMMON_INPUT_PARSER_HPP

#include "particle_record.hpp"
#include <istream>
#include <vector>

namespace common {

[[nodiscard]] std::vector<particle_record> parse_input_stream(std::istream & input, double width, double height);

}  // namespace common

#endif
