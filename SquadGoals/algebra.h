#pragma once

#include "types.h"

class vec2 {
public:
    vec2(f32 x_, f32 y_) : x(x_), y(y_) { }

    inline vec2 operator=(const vec2& other) {
        x = other.x;
        y = other.y;
    }

    inline vec2& operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline vec2& operator-=(const vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline vec2& operator*=(const f32 scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    f32 x, y;
};

inline bool operator==(const vec2& lhs, const vec2& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const vec2& lhs, const vec2& rhs) {
    return !(lhs == rhs);
}

inline vec2 operator+(const vec2& lhs, const vec2& rhs) {
    vec2 ret(lhs.x + rhs.x, lhs.y + rhs.y);
    return ret;
}

inline vec2 operator-(const vec2& lhs, const vec2& rhs) {
    vec2 ret(lhs.x - rhs.x, lhs.y - rhs.y);
    return ret;
}

inline vec2 operator*(const vec2& lhs, const f32 scalar) {
    vec2 ret(lhs.x * scalar, lhs.y * scalar);
    return ret;
}

inline vec2 operator*(const f32 scalar, const vec2& rhs) {
    return rhs * scalar;
}

inline vec2 operator/(const vec2& lhs, const f32 scalar) {
    vec2 ret(lhs.x / scalar, lhs.y / scalar);
    return ret;
}

inline vec2 operator-(const vec2& v) {
    vec2 ret(-v.x, -v.y);
    return ret;
}