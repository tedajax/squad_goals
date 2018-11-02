#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "algebra.h"

struct camera {
    glm::vec3 position = glm::vec3(0, 0, 0);
    f32 phi = 45.f, theta = 0.f, rho = 10.f;

    void move(glm::vec3 movement);
    void move_relative(glm::vec3 movement);
    void orbit(f32 pitch, f32 yaw);

    glm::mat4 view() const;
    glm::mat4 projection() const;
};