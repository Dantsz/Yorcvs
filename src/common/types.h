/**
 * @file types.h
 * @author Dansz
 * @brief 
 * @version 0.1
 * @date 2021-07-27
 * 
 * 
 */
#pragma once
#include <iostream>
#include <cmath>
namespace Yorcvs
{
    /**
     * @brief 2D vector class
     * 
     * @tparam T 
     */
    template<typename T>
    class Vec2
    {
        public:
        constexpr Vec2()
        {
            x = static_cast<T>(0);
            y = static_cast<T>(0);
        }
        constexpr Vec2(T px, T py)
        {
            x = px;
            y = py;
        }
        constexpr Vec2(const Vec2& other)
        {
            x = other.x;
            y = other.y;
        }
        constexpr Vec2(Vec2&& other) noexcept
        {
            x = other.x;
            y = other.y;
        }
        ~Vec2() = default;//trivially destructuted
        constexpr Vec2& operator=(const Vec2& other) = default;
        constexpr Vec2& operator=(Vec2&& other) noexcept = default;
        //Adition
        constexpr Vec2 operator+(const Vec2& other)
        {
            return Vec2(x+other.x,y+other.y);
        }
        constexpr Vec2& operator +=(const Vec2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
        //Subtraction
        constexpr Vec2 operator-(const Vec2& other)
        {
            return Vec2(x-other.x,y-other.y);
        }
        constexpr Vec2& operator -=(const Vec2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
        //Multiplication
        constexpr Vec2 operator*(const Vec2& other)
        {
            return Vec2(x*other.x,y*other.y);
        }
        constexpr Vec2 operator*(const T& other)
        {
            return Vec2(x*other,y*other);
        }

        constexpr Vec2& operator*=(const Vec2& other)
        {
        x *= other.x;
        y *= other.y;
        return *this;
        }

        constexpr Vec2& operator*=(const T& other)
        {
        x *= other;
        y *= other;
        return *this;
        }
        //Division
        constexpr Vec2 operator/(const Vec2& other)
        {
            return Vec2(x/other.x,y/other.y);
        }
        constexpr Vec2 operator/(const T& other)
        {
            return Vec2(x/other,y/other);
        }

        constexpr Vec2& operator/=(const Vec2& other)
        {
        x /= other.x;
        y /= other.y;
        return *this;
        }

        constexpr Vec2& operator/=(const T& other)
        {
        x /= other;
        y /= other;
        return *this;
        }

        constexpr bool operator==(const Vec2& other)
        {
            return(x == other.x && y == other.y);
        }

        constexpr bool operator!=(const Vec2& other)
        {
            return (x != other.x || y != other.y);
        
        }

        /**
         * @brief Returns the norm(length,magnitude) of the vector
         * 
         * @return constexpr T 
         */
        constexpr T norm()
        {
            return static_cast<T>(std::sqrt(x*x + y*y));
        }

        /**
         * @brief Get the Normalized object
         * 
         * @return constexpr Vec2 
         */
        constexpr Vec2 getNormalized()
        {
            return Vec2(x/norm(),y/norm());
        }

        /**
         * @brief Normalize the current vector
         * 
         * @return constexpr Vec2 
         */
        constexpr Vec2& normalize()
        {
            *this = getNormalized();
            return *this;
        }
        /**
         * @brief Returns the dot product between this vector and the other
         * 
         * @param other other vector
         * @return constexpr T 
         */
        constexpr T dotProduct(const Vec2& other)
        {
            return (x*other.x + y*other.y);
        }
    
        T x,y;
    };


    /**
     * @brief 2D rectangle class
     * 
     * @tparam T 
     */
    template<typename T>
    class Rect
    {
        public:
        
        T x, y;
        T w, h;
    };

}

    //Overloads for streams
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Yorcvs::Vec2<T>& vec)
    {
        os << vec.x << ' ' << vec.y;
        return os;
    }

    template<typename T>
    std::istream& operator >> (std::istream& is,Yorcvs::Vec2<T>& vec)
    {
        is >> vec.x;
        is.ignore(1,' ');
        is >> vec.y;
        
        return is;
    }


