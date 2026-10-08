#ifndef DIRECTORY_FILENAME_HPP
#define DIRECTORY_FILENAME_HPP
#include<vector>
namespace common {

}
[[nodiscard]] inline point compute_force(point const & p1, double m1, point const & p2, double m2) {
  (void)p1;
  (void)m1;
  (void)p2;
  (void)m2;
  return point{0.0, 0.0};
}
inline void update_kinematics(point & pos, point & vel, point const & accel, double dt) {
  (void)pos;
  (void)vel;
  (void)accel;
  (void)dt;
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


#endif
