#ifndef AOS_PARTICLE_HPP // if hasnt been defined yet
#define AOS_PARTICLE_HPP // define it

#include <cstddef>

// everything inside gets "aos::" prefix so it doesn't clash with names in SOA
namespace aos{ 
    // we have one object in the simulation with all it's properties stored here
    // this is the "stucture" in "array of struct"
    struct particle{
        std::size_t id = 0; // 0,1,2... in input order after invalid lines are dropped
        double mass = 0.0;  // m 
        double radius = 0.0; // r 
        double x = 0.0; // position x coord
        double y = 0.0; // position y coord
        double vx = 0.0; // velocity x component
        double vy = 0.0; // velocity y component
        double ax = 0.0; // acceleration x component
        double ay = 0.0; // acceleration y component
        bool alive = true; // false when another particle absorbs this one and will remove 
        // all dead ones at once at the end instead of immediately or deleting one from the middle of the list each time
    };
    
    /*
    check input rules from statement: mass and radius must be positive and 
    the whole particle must fit inside the box of given width and height
    */ 
    // [[nodiscard]] tells the compiler that the functions return value must not be ignored, will trigger warning
    [[nodiscard]] bool is_valid_particle(particle const & p, double width, double height); 
}

#endif // close block AOS_PARTICLE_HPP