#include "flatdraw.h"

void flat_draw_context::line(const vec2& from, const vec2& to) {
    r.line(from, to, color);
}

void flat_draw_context::lines(const vec2* points, size_t count) {
    for (auto i = 0; i < count; ++i) {
        line(points[i], points[(i + 1) % count]);
    }
}

void flat_draw_context::circle(const vec2& center, f32 radius, int segments) {
    f32 delta = 360.f / segments;
    for (int i = 0; i < segments; ++i) {
        vec2 p0 = center + math::vec2_from_angle(delta * i) * radius;
        vec2 p1 = center + math::vec2_from_angle(delta * (i + 1)) * radius;
        line(p0, p1);
    }
}