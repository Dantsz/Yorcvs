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

namespace yorcvs
{
/**
 * @brief 2D vector class
 *
 * @tparam T Type should be arithmetic
 */
template <typename T> class Vec2
{
  public:
    constexpr Vec2() : x(static_cast<T>(0)), y(static_cast<T>(0))
    {
    }
    constexpr Vec2(T px, T py) : x(px), y(py)
    {
    }
    constexpr Vec2(const Vec2 &other) : x(other.x), y(other.y)
    {
    }
    constexpr Vec2(Vec2 &&other) noexcept : x(other.x), y(other.y)
    {
    }
    ~Vec2() = default; // trivially destructuted
    constexpr Vec2 &operator=(const Vec2 &other) = default;
    constexpr Vec2 &operator=(Vec2 &&other) noexcept = default;
    // Adition
    constexpr Vec2 operator+(const Vec2 &other) const
    {
        return Vec2(x + other.x, y + other.y);
    }
    constexpr Vec2 &operator+=(const Vec2 &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Subtraction
    constexpr Vec2 operator-(const Vec2 &other) const
    {
        return Vec2(x - other.x, y - other.y);
    }
    constexpr Vec2 &operator-=(const Vec2 &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    // Multiplication
    constexpr Vec2 operator*(const Vec2 &other) const
    {
        return Vec2(x * other.x, y * other.y);
    }
    constexpr Vec2 operator*(const T &other) const
    {
        return Vec2(x * other, y * other);
    }

    constexpr Vec2 &operator*=(const Vec2 &other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    constexpr Vec2 &operator*=(const T &other)
    {
        x *= other;
        y *= other;
        return *this;
    }
    // Division
    constexpr Vec2 operator/(const Vec2 &other) const
    {
        return Vec2(x / other.x, y / other.y);
    }
    constexpr Vec2 operator/(const T &other) const
    {
        return Vec2(x / other, y / other);
    }

    constexpr Vec2 &operator/=(const Vec2 &other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    constexpr Vec2 &operator/=(const T &other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    constexpr bool operator==(const Vec2 &other) const
    {
        return (x == other.x && y == other.y);
    }

    constexpr bool operator!=(const Vec2 &other) const
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
    constexpr Vec2 get_normalized() const
    {
        if (norm() > std::numeric_limits<T>::epsilon())
        {
            return *this / norm();
        }
        return *this;
    }

    /**
     * @brief Normalize the current vector
     *
     * @return constexpr Vec2
     */
    constexpr Vec2 &normalize()
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
    constexpr T dot_product(const Vec2 &other) const
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
template <typename T> class Rect
{
  public:
    constexpr Rect() : x(static_cast<T>(0)), y(static_cast<T>(0)), w(static_cast<T>(0)), h(static_cast<T>(0))
    {
    }
    constexpr Rect(T px, T py, T pw, T ph) : x(px), y(py), w(pw), h(ph)
    {
    }
    constexpr Rect(const Rect &other) : x(other.x), y(other.y), w(other.w), h(other.h)
    {
    }
    constexpr Rect(Rect &&other) noexcept : x(other.x), y(other.y), w(other.w), h(other.h)
    {
    }
    constexpr Rect(const Vec2<T> position, const Vec2<T> dimension)
        : x(position.x), y(position.y), w(dimension.x), h(dimension.y)
    {
    }
    constexpr Rect(const Vec2<T> &position, const Vec2<T> &dimension)
        : x(position.x), y(position.y), w(dimension.x), h(dimension.y)
    {
    }
    ~Rect() = default;
    constexpr Rect &operator=(const Rect &other) = default;
    constexpr Rect &operator=(Rect &&other) noexcept = default;

    constexpr bool operator==(const Rect &other) const
    {
        return (x == other.x && y == other.y && w == other.w && h == other.h);
    }
    constexpr bool operator!=(const Rect &other) const
    {
        return (x != other.x || y != other.y || w != other.w || h != other.h);
    }
    /**
     * @brief Get the parameters of the rectangle
     *
     * @return constexpr Vec2<T>
     */
    constexpr Vec2<T> get_position() const
    {
        return Vec2<T>(x, y);
    }
    /**
     * @brief Get the dimension of the rectangle
     *
     * @return constexpr Vec2<T>
     */
    constexpr Vec2<T> get_dimension() const
    {
        return Vec2<T>(w, h);
    }

    T x, y;
    T w, h;
};

} // namespace yorcvs

// Overloads for streams
template <typename T> std::ostream &operator<<(std::ostream &os, const yorcvs::Vec2<T> &vec)
{
    os << vec.x << ' ' << vec.y;
    return os;
}

template <typename T> std::istream &operator>>(std::istream &is, yorcvs::Vec2<T> &vec)
{
    is >> vec.x;
    is.ignore(1, ' ');
    is >> vec.y;

    return is;
}

template <typename T> std::ostream &operator<<(std::ostream &os, const yorcvs::Rect<T> &rect)
{
    os << rect.x << ' ' << rect.y << ' ' << rect.w << ' ' << rect.h;
    return os;
}
