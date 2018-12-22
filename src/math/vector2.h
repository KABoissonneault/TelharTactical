#pragma once

#include <cmath>

namespace math {
    template<typename T>
    struct vector2 {
        using value_type = T;

        T x, y;

        constexpr auto operator+() const noexcept -> vector2 {
            return *this;
        }
        constexpr auto operator-() const noexcept -> vector2 {
            return vector2{-x, -y};
        }

        constexpr auto operator+=(vector2 other) noexcept -> vector2 & {
            x += other.x;
            y += other.y;
            return *this;
        }
        constexpr auto operator-=(vector2 other) noexcept -> vector2 & {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        constexpr auto operator*=(T other) noexcept -> vector2 & {
            x *= other;
            y *= other;
            return *this;
        }

        constexpr auto operator==(vector2 other) const noexcept -> bool {
            return x == other.x && y == other.y;
        }
        constexpr auto operator!=(vector2 other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    template<typename T>
    inline constexpr auto operator+(vector2<T> lhs, vector2<T> rhs) noexcept -> vector2<T> {
        return lhs += rhs;
    }

    template<typename T>
    constexpr auto operator-(vector2<T> lhs, vector2<T> rhs) noexcept -> vector2<T> {
        return lhs -= rhs;
    }

    template<typename T>
    inline constexpr auto operator*(vector2<T> lhs, typename vector2<T>::value_type rhs) noexcept -> vector2<T> {
        return lhs *= rhs;
    }

    template<typename T>
    inline constexpr auto operator*(typename vector2<T>::value_type lhs, vector2<T> rhs) noexcept -> vector2<T> {
        return rhs *= lhs;
    }

    template<typename T>
    inline constexpr auto operator/(vector2<T> lhs, double rhs) noexcept -> vector2<T> {
        return {lhs.x / rhs, lhs.y / rhs};
    }

    using vector2i = vector2<int>;
    using vector2d = vector2<double>;
    constexpr auto scalar_product(vector2d lhs, vector2d rhs) -> double {
        return lhs.x*rhs.x + lhs.y*rhs.y;
    }

    inline auto magnitude(vector2d v) -> double {
        return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2));
    }
}
