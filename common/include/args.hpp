#ifndef COMMON_PROGARGS_HPP
#define COMMON_PROGARGS_HPP

#include <cstddef>
#include <string>
#include <vector>


namespace common{
    class args{
    private:
    std::string program_name;
    std::size_t iterations=0;
    double width=0.0;
    double height=0.0;
    double timestep=0.0;
    public:
    args(std::vector<std::string>  const & arguments ){
       
    }
    [[nodiscard]]std::string  & program_name(){return program_name;}
    [[nodiscard]]std::size_t  iterations() const{return iterations;}
    [[nodiscard]]double  width() const{return width;}
    [[nodiscard]]double  height() const{return height;}
    [[nodiscard]]double  timestep const{return timestep;}

};
}
 #endif

