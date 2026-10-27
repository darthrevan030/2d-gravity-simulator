#include <cstddef>
#include <sstream>

#include <optional> 

#include <string>
#include "input_parser.hpp"
#include<cmath>
namespace common{
  namespace{
std::optional<particle_record> parse_line(std::string const & line , enclosure const & box,size_t  id){
  std::istringstream iss{line};
  
  
  double mass = 0.0;
  double radius = 0.0;
  double posx=0.0;
  double posy=0.0;
  double velx=0.0;
  double vely=0.0;
  

  if (!(iss >> mass >> radius >> posx >> posy >> velx >> vely)) {
     return std::nullopt;
    
    }   // check if there are 6 parameters on the line

  char extra = 0;
  if (iss >> extra) { 
    
    return std::nullopt; }       // check if there is anything left on teh line


  if (!(std::isfinite(mass) and std::isfinite(radius)  and std::isfinite(posx)  and std::isfinite(posy)  and std::isfinite(velx) and std::isfinite(vely))) { 
    
    return std::nullopt; 
  
  
  }
  if ((mass<=0) or (radius<=0) or (radius>posx) or (posx>box.width-radius) or (radius>posy) or (posy>box.height-radius)){
    return std::nullopt;}
  common::vec2d const vel{velx,vely};
  common::point const pos{posx,posy};
  return particle_record(id, mass, radius,pos,vel );
}}
std::vector<particle_record> parse_input_stream(std::istream & input, enclosure const & box){
  std::vector<particle_record> part;
  std:: string line;
  size_t id=0;
  while (std::getline(input,line)){

    if (auto record= parse_line(line,box ,id)){
    part.push_back(*record);}
      id++;

  }
  return part;






}}
