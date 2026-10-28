#include "particle_system.hpp"
#include "collision_grid.hpp" // common::collision_grid: finds the particles close enough to collide
#include "point.hpp"
#include <gsl/gsl> // gsl::at, gsl::index
#include <algorithm> // std::ranges::max
#include <cmath> // std::sqrt
#include <cstddef>
#include <iterator>
#include <ranges> // std::views::zip
#include <span>
#include <vector>

namespace soa{

    namespace { // helpers only used in this file

        // a particle is alive until it is absorbed; absorbed particles get mass 0 ("tombstone")
        // no real particle has mass 0, because input masses must be positive
        bool is_alive(particle_system const & system, gsl::index k) {
            return gsl::at(system.mass, k) > 0.0;
        }

        // copies every value of particle `from` into position `to` (all 9 vectors, so particle k stays lined up)
        void copy_particle(particle_system & system, gsl::index from, gsl::index to) {
            gsl::at(system.id, to) = gsl::at(system.id, from);
            gsl::at(system.mass, to) = gsl::at(system.mass, from);
            gsl::at(system.radius, to) = gsl::at(system.radius, from);
            gsl::at(system.x, to) = gsl::at(system.x, from);
            gsl::at(system.y, to) = gsl::at(system.y, from);
            gsl::at(system.vx, to) = gsl::at(system.vx, from);
            gsl::at(system.vy, to) = gsl::at(system.vy, from);
            gsl::at(system.ax, to) = gsl::at(system.ax, from);
            gsl::at(system.ay, to) = gsl::at(system.ay, from);
        }

        // cuts every vector down to `count` particles (drops the leftovers at the end)
        void shrink_to(particle_system & system, std::size_t count) {
            system.id.resize(count);
            system.mass.resize(count);
            system.radius.resize(count);
            system.x.resize(count);
            system.y.resize(count);
            system.vx.resize(count);
            system.vy.resize(count);
            system.ax.resize(count);
            system.ay.resize(count);
        }

        // every particle's centre, for the collision grid
        std::vector<common::point> centres(particle_system const & system) {
            std::vector<common::point> out;
            out.reserve(particle_count(system));
            for (auto && [x, y] : std::views::zip(system.x, system.y)) { out.push_back({x, y}); }
            return out;
        }

        // one "row" of the scan: particle i against the LATER particles j that the grid found near it
        // (ascending, so the same order as checking every later particle); merges every overlapping pair;
        // stops early if particle i itself gets absorbed. returns true if anything merged
        bool merge_row(particle_system & system, gsl::index i, std::span<std::size_t const> later) {
            bool merged = false; // nothing merged yet
            for (std::size_t const later_j : later) {
                if (not is_alive(system, i)) { break; } // i was absorbed: the rest of its row is skipped
                auto const j = gsl::narrow_cast<gsl::index>(later_j);
                if (not is_alive(system, j) or not particles_collide(system, i, j)) { continue; }  // skip this pair
                if (first_absorbs_second(system, i, j)) {  // i wins
                    absorb(system, i, j);
                    gsl::at(system.mass, j) = 0.0;  // tombstone: j is gone, keep checking i against the rest
                } else { // j wins
                    absorb(system, j, i);
                    gsl::at(system.mass, i) = 0.0;  // tombstone: i is gone, the next check ends this row
                }
                merged = true;
            }
            return merged;
        }

    } // namespace

    bool particles_collide(particle_system const & system, gsl::index i, gsl::index j) {
        // x particle i minus x of partle j: how far apart in x
        double const dx = gsl::at(system.x, i) - gsl::at(system.x, j);
        // particle y: how far apart in y
        double const dy = gsl::at(system.y, i) - gsl::at(system.y, j);

        // straight line distance between the centers (pythagorars)
        double const distance = std::sqrt(dx * dx + dy * dy);

        // the twp radii added together
        double const radii = gsl::at(system.radius, i) + gsl::at(system.radius, j);
        
        // overlap only if the centers are closer than the radii; exactly touching is not a collision
        return distance < radii;    
    }

    bool first_absorbs_second(particle_system const & system, gsl::index i, gsl::index j){
        // RULE 1: check masses
        double const mass_i = gsl::at(system.mass, i); // mass of particle i
        double const mass_j = gsl::at(system.mass, j); // mass or particle j
        if (mass_i > mass_j) {return true;} // if i is heavier, i wins
        if (mass_i < mass_j) {return false;} // if j is heavier j wins

        // RULE 2 (if masses are equal): check radius of particles, bigger wins
        double const radius_i = gsl::at(system.radius, i); // radius of particlce i
        double const radius_j = gsl::at(system.radius, j); // radius of particle j
        if(radius_i > radius_j) {return true;}
        if(radius_i < radius_j) {return false;}

        // RULE 3: smaller id wins, position=id order so if i is earlier than j 
        return i < j;
    }

    void absorb(particle_system & system, gsl::index absorber, gsl::index absorbed) { 
        // absorber's mass += absorbed mass
        gsl::at(system.mass, absorber) += gsl::at(system.mass, absorbed);
        gsl::at(system.vx, absorber) += gsl::at(system.vx, absorbed); // x vel add
        gsl::at(system.vy, absorber) += gsl::at(system.vy, absorbed); // y vel add
        // -- radius, x, y are not touched so the absorber keeps its own radius and position
    }

    void remove_absorbed(particle_system & system) {
        gsl::index kept    = 0; // how many survivors so far = where the next one goes
        gsl::index const n = std::ssize(system.mass);
        for (gsl::index k = 0; k < n; ++k) { // every position, in order
            if (is_alive(system, k)) {
                copy_particle(system, k, kept); // slide the survivor down to the next free spot
                ++kept;
            }
        }
        shrink_to(system, static_cast<std::size_t>(kept)); // drop everything after the last survivor
    }

    void resolve_collisions(particle_system & system) {
        if (particle_count(system) < 2) { return; } // nothing can collide
        // nothing moves during the pass, so a grid built once stays valid for all of it
        auto const points = centres(system);
        common::collision_grid const grid{points, std::ranges::max(system.radius)};
        std::vector<std::size_t> later; // reused for every row
        bool any_merged = false;
        for (std::size_t i = 0; i < particle_count(system); ++i) { // every row, in id order
            grid.later_neighbours(i, later);
            if (merge_row(system, gsl::narrow_cast<gsl::index>(i), later)) { any_merged = true; }
        }
        if (any_merged) { remove_absorbed(system); } // only do the removal pass if something merged
    }
}