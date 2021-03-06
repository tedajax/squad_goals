#pragma once

#include "types.h"
#include <limits>
#include <glm/vec3.hpp>

struct b2Vec2;

class vec2 {
public:
    vec2() : x(0), y(0) { }
    vec2(const vec2& other) : x(other.x), y(other.y) {}
    vec2(const b2Vec2& b2vec);
    vec2(vec2&& other) : x(other.x), y(other.y) {}
    vec2(f32 x, f32 y) : x(x), y(y) { }
    vec2(int x, int y) : x((f32)x), y((f32)y) { }

    operator b2Vec2() const;
    operator glm::vec3() const;

    inline vec2 operator=(const vec2& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    inline vec2 operator=(vec2&& other) {
        x = other.x;
        y = other.y;
        return *this;
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

    inline vec2& operator*=(f32 scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline vec2& operator/=(f32 scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    inline vec2& operator/=(int scalar) {
        x /= (f32)scalar;
        y /= (f32)scalar;
        return *this;
    }

    inline f32 len() const{
        return std::sqrt(x *x + y * y);
    }

    inline f32 len2() const {
        return x * x + y * y;
    }

    inline f32 dot(const vec2& other) const {
        return x * other.x + y * other.y;
    }

    void decompose(vec2& direction, f32& magnitude) {
        magnitude = len();
        if (magnitude > 0) {
            direction = vec2(x / magnitude, y / magnitude);
        }
        else {
            direction = vec2::ZERO;
        }
    }

    vec2 normalize() {
        f32 l = len();
        if (l > 0) {
            *this /= l;
        }
        return *this;
    }

    static vec2 normalize(const vec2& v) {
        f32 l = v.len();
        if (l > 0) {
            vec2 ret(v.x / l, v.y / l);
            return ret;
        }
        else {
            return ZERO;
        }
    }

    static vec2 perpendicular(const vec2& v) {
        return vec2(-v.y, v.x);
    }

    static f32 dot(const vec2& a, const vec2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static f32 dist(const vec2& a, const vec2& b) {
        return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    }

    static vec2 left(const vec2& a, const vec2& b) {
        return (a.x <= b.x) ? a : b;
    }

    static vec2 right(const vec2& a, const vec2& b) {
        return (a.x >= b.x) ? a : b;
    }

    static vec2 top(const vec2& a, const vec2& b) {
        return (a.y <= b.y) ? a : b;
    }

    static vec2 bottom(const vec2& a, const vec2& b) {
        return (a.y >= b.y) ? a : b;
    }

    static vec2 clamp_to_segment(const vec2& v, const vec2& a, const vec2& b) {
        // special rules for vertical line
        if (a.x - b.x == 0) {
            vec2 t = top(a, b), b = bottom(a, b);
            if (v.y < t.y) {
                return t;
            }
            else if (v.y > b.y) {
                return b;
            }
        }
        else {
            vec2 l = left(a, b), r = right(a, b);
            if (v.x < l.x) {
                return l;
            }
            else if (v.x > r.x) {
                return r;
            }
        }

        return v;
    }

    // if v is on segment between a-b then return v, otherwise return other
    static vec2 on_segment_or_other(const vec2& v, const vec2& a, const vec2& b, const vec2& other) {
        // special rules for vertical line
        if (a.x - b.x == 0) {
            vec2 t = top(a, b), b = bottom(a, b);
            if (v.y < t.y) {
                return other;
            }
            else if (v.y > b.y) {
                return other;
            }
        }
        else {
            vec2 l = left(a, b), r = right(a, b);
            if (v.x < l.x) {
                return other;
            }
            else if (v.x > r.x) {
                return other;
            }
        }

        return v;
    }

    static f32 angle_between(const vec2& a, const vec2& b);

    void limit(f32 mag) {
        f32 l = len();
        if (l > mag) {
            f32 s = (mag / l);
            x *= s;
            y *= s;
        }
    }

    static vec2 ZERO;
    static vec2 ONE;
    static vec2 RIGHT;
    static vec2 UP;

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

inline vec2 operator*(const vec2& lhs, f32 scalar) {
    vec2 ret(lhs.x * scalar, lhs.y * scalar);
    return ret;
}

inline vec2 operator*(f32 scalar, const vec2& rhs) {
    return rhs * scalar;
}

inline vec2 operator/(const vec2& lhs, f32 scalar) {
    vec2 ret(lhs.x / scalar, lhs.y / scalar);
    return ret;
}

inline vec2 operator-(const vec2& v) {
    vec2 ret(-v.x, -v.y);
    return ret;
}

namespace math {
    const f32 PI = 3.1415927410125732421875;
    const f32 TWO_PI = PI * 2;
    const f32 PI_OVER_2 = PI / 2;

    const f32 RAD_TO_DEG = 180.f / PI;
    const f32 DEG_TO_RAD = PI / 180.f;

    const f32 EPSILON = std::numeric_limits<f32>::epsilon();

    inline static f32 abs(f32 v) {
        return std::fabsf(v);
    }

    inline f32 cos(f32 degrees) {
        return std::cosf(degrees * DEG_TO_RAD);
    }

    inline f32 sin(f32 degrees) {
        return std::sinf(degrees * DEG_TO_RAD);
    }

    inline f32 tan(f32 degrees) {
        return std::tanf(degrees * DEG_TO_RAD);
    }

    inline f32 acos(f32 v) {
        return std::acosf(v) * RAD_TO_DEG;
    }

    inline f32 asin(f32 v) {
        return std::asinf(v) * RAD_TO_DEG;
    }

    inline f32 atan(f32 v) {
        return std::atan(v) * RAD_TO_DEG;
    }

    inline f32 atan2(f32 dy, f32 dx) {
        return std::atan2f(dy, dx) * RAD_TO_DEG;
    }

    inline f32 min(f32 a, f32 b) {
        return (a < b) ? a : b;
    }

    inline f32 max(f32 a, f32 b) {
        return (a > b) ? a : b;
    }

    inline f32 sign(f32 v) {
        return (v >= 0) ? 1.f : -1.f;
    }

    inline f32 pow(f32 f, f32 p) {
        return std::powf(f, p);
    }

    inline f32 exp(f32 p) {
        return std::expf(p);
    }

    inline f32 log(f32 f) {
        return std::logf(f);
    }

    inline f32 log(f32 f, f32 b) {
        return std::logf(f) / std::logf(b);
    }

    inline f32 log10(f32 f) {
        return std::log10f(f);
    }

    inline f32 log2(f32 f) {
        return std::log2f(f);
    }

    inline f32 floor(f32 v) {
        return std::floorf(v);
    }

    inline f32 ceil(f32 v) {
        return std::ceilf(v);
    }

    inline f32 round(f32 v) {
        return std::roundf(v);
    }

    inline f32 sqrt(f32 v) {
        return std::sqrtf(v);
    }

    inline int floor_int(f32 v) {
        return (int)floor(v);
    }

    inline int ceil_int(f32 v) {
        return (int)ceil(v);
    }

    inline int round_int(f32 v) {
        return (int)round(v);
    }
    
    inline f32 clamp(f32 v, f32 min, f32 max) {
        if (v < min) {
            return min;
        }
        else if (v > max) {
            return max;
        }
        else {
            return v;
        }
    }

    inline f32 clamp01(f32 v) {
        return clamp(v, 0, 1);
    }

    inline static bool approx(f32 a, f32 b) {
        return abs(b - a) * max(0.000001f * max(abs(a), abs(b)), EPSILON * 8);
    }

    inline static bool approx_zero(f32 v) {
        return approx(v, 0);
    }
    
    inline f32 repeat(f32 v, f32 max) {
        return clamp(v - floor(v / max) * max, 0.f, max);
    }

    inline f32 ping_pong(f32 v, f32 len) {
        v = repeat(v, len * 2.f);
        return len - abs(v - len);
    }

    inline vec2 vec2_from_angle(f32 degrees) {
        return vec2(cos(degrees), sin(degrees));
    }

    inline f32 angle_from_vec2(const vec2& normVec) {
        return repeat(atan2(normVec.y, normVec.x), 360.f);
    }

    inline f32 lerp(f32 a, f32 b, f32 t) {
        return a + (b - a) * t;
    }
    inline f32 lerp_angle(f32 a, f32 b, f32 t) {
        f32 delta = repeat(b - a, 360.f);
        if (delta > 180.f) {
            delta -= 360.f;
        }
        return a + delta * t;
    }

    // determines scalar value between [0-1] of v between a and b
    inline f32 inv_lerp(f32 a, f32 b, f32 v) {
        if (a != b) {
            return clamp01((v - a) / (b - a));
        }
        return 0.f;
    }

    inline f32 delta_angle(f32 a, f32 b) {
        f32 delta = repeat(b - a, 360.f);
        if (delta > 180.f) {
            delta -= 360.f;
        }
        return delta;
    }

    inline f32 move_to(f32 from, f32 to, f32 delta) {
        if (abs(to - from) <= delta) {
            return to;
        }
        return from + sign(to - from) * delta;
    }

    inline f32 rotate_to(f32 from, f32 to, f32 delta) {
        f32 da = delta_angle(from, to);
        if (-delta < da && da < delta) {
            return to;
        }
        return move_to(from, from + da, delta);
    }

    inline f32 smooth_step(f32 from, f32 to, f32 t) {
        t = clamp01(t);
        t = -2.f * t * t * t + 3.f * t * t;
        return to * t + from * (1.f - t);
    }

    inline f32 fade(f32 t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    inline f32 grad(int hash, f32 x, f32 y, f32 z) {
        int h = hash & 15;
        f32 u = (h < 8) ? x : y;
        f32 v = (h < 4) ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    inline f32 smooth_damp(f32 from, f32 to, f32& velocity, f32 time, f32 maxSpeed = std::numeric_limits<f32>::infinity(), f32 dt = 1.f / 60.f) {
        // from Unity math reference implementation
        // https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Mathf.cs
        // originally from Game Programming Gems 4 Chapter 1.10
        
        time = max(0.0001f, time);
        f32 omega = 2.f / time;

        f32 x = omega * dt;
        f32 exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
        f32 delta = to - from;
        f32 origTo = to;

        f32 maxDelta = maxSpeed * time;
        delta = clamp(delta, -maxDelta, maxDelta);
        to = from - delta;

        f32 temp = (velocity + omega * delta) * dt;
        velocity = (velocity - omega * temp) * exp;
        f32 ret = to + (delta + temp) * exp;

        // prevent overshoots
        if (origTo - from > 0.0f == ret > origTo) {
            ret = origTo;
            velocity = (ret - origTo) / dt;
        }

        return ret;
    }

    inline f32 smooth_damp_angle(f32 from, f32 to, f32& velocity, f32 time, f32 maxSpeed = std::numeric_limits<f32>::infinity(), f32 dt = 1.f / 60.f) {
        f32 target = from + delta_angle(from, to);
        return smooth_damp(from, target, velocity, time, maxSpeed, dt);
    }
}

struct aabb {
    vec2 botLeft;
    vec2 topRight;

    inline f32 left() const { return botLeft.x; }
    inline f32 right() const { return topRight.x; }
    inline f32 top() const { return topRight.y; }
    inline f32 bottom() const { return botLeft.y; }
    inline vec2 center() const { return (botLeft + topRight) / 2; }
    inline vec2 dimensions() const { return vec2(math::abs(topRight.x - botLeft.x), math::abs(topRight.y - botLeft.y)); }

    inline bool contains(vec2 pt) {
        return pt.x >= left() && pt.x <= right() && pt.y >= bottom() && pt.y <= top();
    }

    inline void move(vec2 amount) {
        botLeft += amount;
        topRight += amount;
    }

    inline static f32 distance(const aabb& a, const aabb& b) {
        f32 dy1 = math::max(a.bottom() - b.top(), 0.f);
        f32 dy2 = math::max(b.bottom() - a.top(), 0.f);
        f32 dy = math::max(dy1, dy2);

        f32 dx1 = math::max(a.left() - b.right(), 0.f);
        f32 dx2 = math::max(b.left() - a.right(), 0.f);
        f32 dx = math::max(dx1, dx2);

        return math::sqrt(dx * dx + dy * dy);
    }

    inline static aabb create_from_center(vec2 center, vec2 dimensions) {
        auto half = dimensions / 2;
        return aabb{ center - half, center + half };
    }
};

