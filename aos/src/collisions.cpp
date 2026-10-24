#include "particle_system.hpp"
#include "collision_rules.hpp"

namespace aos{
void particle_system::process_collisions()
{
  bool collision_found = true;
  while (collision_found){
    collision_found = false:

    for(std::size_t i=0; i< particles_.size(); ++i){
      for(std::size_t j=i+1; j< particles_.size(); ++j){
        if(!common::checkcollision(particles_[i].pos, particles_[i].radius, particles_[j].pos, particles_[j].radius))
        {
          continue;
        }

        std::size_t absorber =i;
        std::size_t absorbed =j;

        if(particles_[j].mass > paricles_[i].mass){
          absorber = j;
          absorbed = i;
        }
        else if(particles_[i].mass==particles_[j] && particles_[j].radius > particles_[i].radius){
          absorber = j;
          absorbed = i; 
        }

        common::merge_particles(particles_[absorber], particles_[absorbed]);
        particles_.erase(particles_.begin() + absorbed);
        collision_found = true;
        break;
      }
      if(collision_found){
        break;
      }
    }
  }
}
}
  
