#pragma once

#include "algebra.h"
#include <vector>
#include <limits>

enum class path_dir {
    kCW,
    kCCW
};

class path {
public:
    path(path_dir dir, f32 width, std::initializer_list<vec2> pts);

    // find nearest point on path to point p
    vec2 nearest(const vec2& p, vec2& direction);
    // how far away is p from the path
    f32 distance(const vec2& p);

    inline const std::vector<vec2>& path_points() const { return points; }
    inline const f32 path_width() const { return width; }

private:
    std::vector<vec2> points;
    path_dir dir;
    f32 width;
};