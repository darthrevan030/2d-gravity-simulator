#ifndef COMMON_VECTOR_HPP
#define COMMON_VECTOR_HPP

#include "point.hpp"
#include <cmath>
namespace common{
    class vec2d{
        private:
        double x;
        double y;
        public:
        constexpr vec2d(double x_val=0,double y_val=0):
            x(x_val), y(y_val){

        }
        [[nodiscard]] constexpr double getx() const noexcept{return x;}
        [[nodiscard]] constexpr double gety() const noexcept{return y;}
        [[nodiscard]]  double norm2d() const noexcept {return sqrt(x*x+y*y);}


    };

[[nodiscard]] constexpr vec2d operator-(point const & a, point const & b) noexcept{
    return vec2d{a.getx()-b.getx(),a.gety()-b.gety()};
}
