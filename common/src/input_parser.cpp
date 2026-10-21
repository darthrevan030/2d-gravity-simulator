#include <sstream>

#include <optional> 

#include <string>
#include "input_parser.hpp"
#include<cmath>
namespace common{
  namespace{
std::optional<particle_record> parse_line(std::string const & line , double width, double height){
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
  if (iss >> extra) { return std::nullopt; }       // check if there is anything left on teh line


  if (!(std::isfinite(mass) && std::isfinite(radius)  && std::isfinite(posx)  && std::isfinite(posy)  && std::isfinite(velx) && std::isfinite(vely))) { 
    
    
    return std::nullopt; 
  
  
  }
  if ((mass<=0)||(radius<=0)||(radius>posx)||(posx>width-radius)||(radius>posy)||(posy>height-radius)){return std::nullopt;}
  return particle_record(mass, radius, posx, posy, velx, vely);
}}
std::vector<particle_record> parse_input_stream(std::istream & input, double width, double height){
  std::vector<particle_record> part;
  std:: string line;
  while (std::getline(input,line)){
    if (auto record= parse_line(line,width ,height)){
    part.push_back(*record);}
  }
  return part;






}}
