#include "particle.hpp"

namespace aos{
    bool is_valid_particle(particle const & p, double width, double height){
        bool const positive_mass = p.mass > 0.0; // mass must be positive
        bool const positive_radius = p.radius > 0.0; // radius must be positive

        // x must be btwn the radius and (width-radius), so the particle does not stick out
        bool const x_inside = p.x >= p.radius and p.x <= width - p.radius;
        // same for y
        bool const y_inside = p.y >= p.radius and p.y <= height - p.radius;

        
        // only valid if all four hold
        return positive_mass and positive_radius and x_inside and y_inside;

    }
} 

/*
explination notes:
all in box of 100x50 wth radius-2 part
example for p.x/p.y >= p.radius must hold
- say A is at center x=50 with radius 2, its leftmost point is 50-2=48 which is inside the wall
- say B is at center x=2 with radius 2, its leftmost point is 2-2=0 which is inside the walls
- say C is at center x=1 with radius 2, its leftmost point is 1-2=-1, one unit of the particle is outside the box through the wall, not ok
x_past_left_limit test: x=1.9 with radius 2, its leftmost point is 1.9-2=-0.1, leftmost point is -0.1 is just outside the box

example for p.x/p.y <= width-radius
rightmost point is at x+radius which cannot go past width
- center at x=98 with radius 2, rightmost point is 98+2=100 which is exactly touching
- center at x=99 with radius 2, rightmost point is 99+2=101, one unit past the wall


given D center x=99, y=10:
left wall: x>=radius 99>=2 good
right wall: x<=width-radius 99<=100-2 99!<= 98 no good
bottom wall: y>=radius 10>=2 good
top wall: y<=height-radius 10<=50-2 10<=48 good
fails because right wall
all must hold otherwise fail!
*/