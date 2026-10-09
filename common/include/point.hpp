#ifndef COMMON_POINT_HPP
#define COMMON_POINT_HPP

#include <format>
#include <istream>
#include <string_view>

namespace common {

  class point {
  public:
    double x = 0.0;
    double y = 0.0;
    [[nodiscard]] constexpr double getx() const noexcept { return x; }
    [[nodiscard]] constexpr double gety() const noexcept { return y; }
  };

  inline std::istream & operator>>(std::istream & is, point & p) {
    is >> p.x >> p.y;
    return is;
  }

}  // namespace common

template <> struct std::formatter<common::point> : public std::formatter<std::string_view> {
  auto format(common::point const & p, std::format_context & ctx) const{
    std::string const text = std::format("{} {}", p.x, p.y);
    return std::formatter<std::string_view>::format(text, ctx);
  }
};

#endif  // COMMON_POINT_HPP
