#pragma once

#include "algebra.h"

#include <vector>

//struct quad_node {
//    int id = 0;
//    vec2 point = vec2::ZERO;
//    int childIds[4] = { -1, -1, -1, -1 };
//};
//
//class quad_tree {
//    std::vector<quad_node> nodes;
//    
//    void insert(const vec2& pt) {
//        if (nodes.size() == 0) {
//            nodes.push_back(quad_node{ 0, vec2::ZERO });
//        }
//
//
//    }
//};

struct quad {
    aabb bounds;

    int topLeft;
    int topRight;
    int bottomLeft;
    int bottomRight;
};

class quad_tree {
    quad_tree(f32 size) {
        quads.push_back(quad { aabb(, -1, -1, -1, -1 });


    }

    void subdivide(quad* current, f32 minSize) {
        if (current->)
    }


    std::vector<quad> quads;
};

