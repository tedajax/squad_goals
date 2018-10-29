#include "flowfield.h"

flow_field::flow_field(f32 worldWidth, f32 worldHeight, f32 cellSize)
    : worldWidth(worldWidth),
    worldHeight(worldHeight),
    cellSize(cellSize),
    cellWidth(math::ceil_int(worldWidth / cellSize)),
    cellHeight(math::ceil_int(worldHeight / cellSize))
{
    vectors.resize(cellWidth * cellHeight);
}

void flow_field::perlin_angles(const perlin_gen& perlin, f32 scale, f32 z /* = 0.f */) {
    for (int i = 0; i < cellWidth; ++i) {
        for (int j = 0; j < cellHeight; ++j) {
            f32 v = perlin.noise((f32)i / cellWidth * scale, (f32)j / cellHeight * scale, z);
            set(i, j, math::vec2_from_angle(v * 720.f));
        }
    }
}

void flow_field::set(int cellX, int cellY, vec2 vec) {
    int i = index(cellX, cellY);
    if (i >= 0) {
        vectors[i] = vec;
    }
}

vec2 flow_field::get(int cx, int cy) const {
    int i = index(cx, cy);
    if (i >= 0) {
        return vectors[i];
    }
    return vec2::ZERO;
}

vec2 flow_field::get(f32 x, f32 y) const {
    if (x < 0 || x > worldWidth || y < 0 || y > worldHeight) {
        return vec2::ZERO;
    }
    return vectors[index((int)(x / cellSize), (int)(y / cellSize))];
}

vec2 flow_field::cell_center(int cx, int cy) {
    return vec2(cx * cellSize + cellSize / 2, cy * cellSize + cellSize / 2);
}

int flow_field::width() const { return cellWidth; }
int flow_field::height() const { return cellHeight; }
f32 flow_field::cell_size() const { return cellSize; }

inline int flow_field::index(int x, int y) const {
    if (x >= 0 && x < cellWidth && y >= 0 && y < cellHeight) {
        return y * cellWidth + x;
    }
    return -1;
}