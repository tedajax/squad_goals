#pragma once

#include "algebra.h"
#include "perlin.h"
#include <vector>

class flow_field {
public:
    flow_field(f32 worldWidth, f32 worldHeight, f32 cellSize);
    void perlin_angles(const perlin_gen& perlin, f32 scale, f32 z = 0.f);
    void set(int cellX, int cellY, vec2 vec);
    vec2 get(int cx, int cy) const;
    vec2 get(f32 x, f32 y) const;
    vec2 cell_center(int cx, int cy);

    int width() const;
    int height() const;
    f32 cell_size() const;

private:
    inline int index(int x, int y) const;

    f32 worldWidth;
    f32 worldHeight;
    f32 cellSize;
    int cellWidth;
    int cellHeight;
    std::vector<vec2> vectors;
};