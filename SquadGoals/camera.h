#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct camera {
    glm::vec3 position;
    glm::quat rotation = glm::quat();

    void move_relative(glm::vec3 movement);
    void rotate(glm::quat rotation);

    glm::mat4 look_at() const;
    glm::mat4 projection() const;
};