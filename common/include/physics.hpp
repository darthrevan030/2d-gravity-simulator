#ifndef COMMON_PHYSICS_HPP
#define COMMON_PHYSICS_HPP

#include<vector>
#include "point.hpp"
#include "vector.hpp"
#include "constants.hpp"
#include <cmath>
namespace common {


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
  vel.setx(vel.getx()+accel.getx()*dt);
  vel.sety(vel.gety()+accel.gety()*dt);
  pos.setx(pos.getx()+vel.getx()*dt);
  pos.sety(pos.gety()+vel.gety()*dt);

}

/**
 * @brief Applies wall collision rebounds (left, right, bottom, top boundaries).
 * @details Checks <= / >= boundary limits, clamps position inside [r, limit - r],
 *          and reverses velocity direction if moving toward the wall.
 */
inline void apply_rebound(point & pos, vec2d & vel, double radius, double width, double height) {

  if ((pos.getx()-radius)<=0.0){
    pos.setx(0.0+radius);
    if (vel.getx()<0.0){

      vel.setx(-1*vel.getx());}

}
if ((pos.getx()+radius)>=width){
    pos.setx(width-radius);
    if (vel.getx()>0.0){
    
      vel.setx(-1*vel.getx());

}}
if ((pos.gety()+radius)>=height){
    pos.sety(height-radius);
    if (vel.gety()>0){

    vel.sety(-1*vel.gety());
    }
}
if ((pos.gety()-radius)<=0.0){
    pos.sety(0.0+radius);
    if (vel.gety()<0){

    vel.sety(-1*vel.gety());
    }
}}

}
#endif
