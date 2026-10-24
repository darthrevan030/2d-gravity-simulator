#include "particle_system.hpp"
#include "physics.hpp"

namespace aos{
std::vector<common::vec2d>
particle_system::compute_accelerations() const
{
  std::vector<common::vec2d> acc(
    particles_.size(),
    common::vec2d{}
  );
  for(std::size_t i=0; i< particles_.size(); ++i)
    {
      for(std::size_t j=i+1; j< particles_.size(); ++j)
        {
          auto force = common::compute_force(particles_[i].pos, particles_[i].masss, particles_[j].pos, particles_[j].mass);
          acc[i] = acc[i] + force * (1.0/particles_[i].mass);
          acc[j] = acc[j] + force * (1.0/particles_[j].mass); 
        }
    }
  return acc; 
}
}
