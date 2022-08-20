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
#include <cmath>
#include <iostream>

namespace yorcvs {
/**
 * @brief 2D vector class
 *
 * @tparam T Type should be arithmetic
 */
template <typename T>
class vec2 {
public:
    constexpr vec2()
        : x(static_cast<T>(0))
        , y(static_cast<T>(0))
    {
    }
    constexpr vec2(T px, T py) noexcept
        : x(px)
        , y(py)
    {
    }
    constexpr vec2(const vec2& other)
        : x(other.x)
        , y(other.y)
    {
    }
    constexpr vec2(vec2&& other) noexcept
        : x(other.x)
        , y(other.y)
    {
    }
    template <typename otherT>
    explicit constexpr vec2(const vec2<otherT>& other) noexcept
        : x(static_cast<T>(other.x))
        , y(static_cast<T>(other.y))
    {
    }
    ~vec2() = default; // trivially destructuted
    constexpr vec2& operator=(const vec2& other) = default;
    constexpr vec2& operator=(vec2&& other) noexcept = default;
    // Adition
    constexpr vec2 operator+(const vec2& other) const
    {
        return vec2(x + other.x, y + other.y);
    }
    constexpr vec2& operator+=(const vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Subtraction
    constexpr vec2 operator-(const vec2& other) const
    {
        return vec2(x - other.x, y - other.y);
    }
    constexpr vec2& operator-=(const vec2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    // Multiplication
    constexpr vec2 operator*(const vec2& other) const noexcept
    {
        return vec2(x * other.x, y * other.y);
    }
    constexpr vec2 operator*(const T& other) const noexcept
    {
        return vec2(x * other, y * other);
    }

    constexpr vec2& operator*=(const vec2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    constexpr vec2& operator*=(const T& other)
    {
        x *= other;
        y *= other;
        return *this;
    }
    // Division
    constexpr vec2 operator/(const vec2& other) const
    {
        return vec2(x / other.x, y / other.y);
    }
    constexpr vec2 operator/(const T& other) const
    {
        return vec2(x / other, y / other);
    }

    constexpr vec2& operator/=(const vec2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    constexpr vec2& operator/=(const T& other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    constexpr bool operator==(const vec2& other) const
    {
        return (x == other.x && y == other.y);
    }

    constexpr bool operator!=(const vec2& other) const
    {
        return (x != other.x || y != other.y);
    }

    /**
     * @brief Returns the norm(length,magnitude) of the vector
     *
     * @return constexpr T
     */
    constexpr T norm() const
    {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    /**
     * @brief Get the Normalized object
     *
     * @return constexpr Vec2
     */
    constexpr vec2 get_normalized() const
    {
        if (norm() > std::numeric_limits<T>::epsilon()) {
            return *this / norm();
        }
        return *this;
    }

    /**
     * @brief Normalize the current vector
     *
     * @return constexpr Vec2
     */
    constexpr vec2& normalize()
    {
        *this = get_normalized();
        return *this;
    }
    /**
     * @brief Returns the dot product between this vector and the other
     *
     * @param other other vector
     * @return constexpr T
     */
    constexpr T dot_product(const vec2& other) const
    {
        return (x * other.x + y * other.y);
    }

    T x, y;
};

/**
 * @brief 2D rectangle class
 *
 * @tparam T type should be arithmetic
 */
template <typename T>
class rect {
public:
    constexpr rect()
        : x(static_cast<T>(0))
        , y(static_cast<T>(0))
        , w(static_cast<T>(0))
        , h(static_cast<T>(0))
    {
    }
    constexpr rect(T px, T py, T pw, T ph)
        : x(px)
        , y(py)
        , w(pw)
        , h(ph)
    {
    }
    constexpr rect(const rect& other)
        : x(other.x)
        , y(other.y)
        , w(other.w)
        , h(other.h)
    {
    }
    constexpr rect(rect&& other) noexcept
        : x(other.x)
        , y(other.y)
        , w(other.w)
        , h(other.h)
    {
    }
    constexpr rect(const vec2<T> position, const vec2<T> dimension)
        : x(position.x)
        , y(position.y)
        , w(dimension.x)
        , h(dimension.y)
    {
    }
    constexpr rect(const vec2<T>& position, const vec2<T>& dimension)
        : x(position.x)
        , y(position.y)
        , w(dimension.x)
        , h(dimension.y)
    {
    }
    ~rect() = default;
    constexpr rect& operator=(const rect& other) = default;
    constexpr rect& operator=(rect&& other) noexcept = default;

    constexpr bool operator==(const rect& other) const
    {
        return (x == other.x && y == other.y && w == other.w && h == other.h);
    }
    constexpr bool operator!=(const rect& other) const
    {
        return (x != other.x || y != other.y || w != other.w || h != other.h);
    }
    /**
     * @brief Get the parameters of the rectangle
     *
     * @return constexpr Vec2<T>
     */
    constexpr vec2<T> get_position() const
    {
        return vec2<T>(x, y);
    }
    /**
     * @brief Get the dimension of the rectangle
     *
     * @return constexpr Vec2<T>
     */
    constexpr vec2<T> get_dimension() const
    {
        return vec2<T>(w, h);
    }

    constexpr bool contains(const yorcvs::vec2<T>& point)
    {
        return (point.x >= x) && (point.x <= x + w) && (point.y >= y) && (point.y <= y + h);
    }

    T x, y;
    T w, h;
};

} // namespace yorcvs

// Overloads for streams
template <typename T>
std::ostream& operator<<(std::ostream& os, const yorcvs::vec2<T>& vec)
{
    os << vec.x << ' ' << vec.y;
    return os;
}

template <typename T>
std::istream& operator>>(std::istream& is, yorcvs::vec2<T>& vec)
{
    is >> vec.x;
    is.ignore(1, ' ');
    is >> vec.y;

    return is;
}
template <typename T>
std::ostream& operator<<(std::ostream& os, const yorcvs::rect<T>& rect)
{
    os << rect.x << ' ' << rect.y << ' ' << rect.w << ' ' << rect.h;
    return os;
}
