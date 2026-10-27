#ifndef COMMON_ENCLOSURE_HPP
#define COMMON_ENCLOSURE_HPP

namespace common {

/// Dimensions of the closed simulation enclosure. Both values must be positive.
struct enclosure {
  double width;
  double height;
};

}  // namespace common

#endif  // COMMON_ENCLOSURE_HPP