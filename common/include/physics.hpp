
#ifndef DIRECTORY_FILENAME_HPP
#define DIRECTORY_FILENAME_HPP
#include<vector>

#include <cmath>
namespace common {

}
[[nodiscard]] inline vec2d compute_force(point const & p1, double m1, point const & p2, double m2) {
  vec2d difvec=p2-p1;
  double const dist_sq = difvec.getx()*difvec.getx() + difvec.gety()*difvec.gety();
  if (dist_sq<EPSILON*EPSILON){
    return vec2d{0,0};}

  double vec3norm=dist_sq*(std::sqrt(dist_sq));

  double scalar1=G*(1/vec3norm)*m1*m2;

  return difvec*scalar1;
}
inline void update_kinematics(point & pos, vec2d & vel, vec2d const & accel, double dt) {
  vec2d acceleration=
}

/**
 * @brief Applies wall collision rebounds (left, right, bottom, top boundaries).
 * @details Checks <= / >= boundary limits, clamps position inside [r, limit - r],
 *          and reverses velocity direction if moving toward the wall.
 */
inline void apply_rebound(point & pos, point & vel, double radius, double width, double height) {
  (void)pos;
  (void)vel;
  (void)radius;
  (void)width;
  (void)height;
}

}
#endif
