#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct camera {
    glm::vec3 position;
    glm::quat rotation;

    void move_relative(glm::vec3 movement);

    glm::mat4x4 look_at() const;
    glm::mat4x4 projection() const;
};