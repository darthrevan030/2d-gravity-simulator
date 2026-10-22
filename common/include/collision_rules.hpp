#ifndef COMMON_COLLISION_RULES_HPP
#define COMMON_COLLISION_RULES_HPP

#include "particle_record.hpp"
#include "point.hpp"
#include <cmath>
namespace common{
    [[nodiscard]] inline bool checkcollision(point p1,double r1,point p2, double r2){
        if ((r1+r2)>sqrt((p1.x-p2.x)*(p1.x-p2.x))+((p1.y-p2.y)*(p1.y-p2.y))){
            return true;
        }
    
    return false
}
inline void merge_particles(particle_record & absorber, particle_record const & absorbed) {
  (void)absorber;
  (void)absorbed;
}}
#endif
