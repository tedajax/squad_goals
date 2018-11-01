#pragma once

#include "renderer.h"
#include "algebra.h"

class flat_draw_context {
public:
    flat_draw_context(renderer& r) : r(r), color(1, 1, 1, 1) { }

    void set_color(f32 r, f32 g, f32 b, f32 a = 1.f) {
        color = glm::vec4(r, g, b, a);
    }

    void set_color_bytes(u8 r, u8 g, u8 b, u8 a = 255) {
        set_color(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
    }

    void line(const vec2& from, const vec2& to);
    void lines(const vec2* points, size_t count);
    void circle(const vec2& center, f32 radius, int segments = 11);

private:
    renderer& r;
    glm::vec4 color;
};