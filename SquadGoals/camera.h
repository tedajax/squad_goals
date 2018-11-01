#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "algebra.h"

struct camera {
    glm::vec3 position;
    glm::quat rotation = glm::angleAxis(-90.f * math::DEG_TO_RAD, glm::vec3(1, 0, 0));

    void move(glm::vec3 movement);
    void move_relative(glm::vec3 movement);
    void rotate(glm::quat rotation);

    glm::mat4 look_at() const;
    glm::mat4 projection() const;
};