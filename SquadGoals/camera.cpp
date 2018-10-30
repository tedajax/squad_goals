#include "camera.h"

void camera::move_relative(glm::vec3 movement) {
    position += movement * rotation;
}

glm::mat4x4 camera::look_at() const {
    glm::vec3 center = position + glm::vec3(0.f, 0.f, -1.f) * rotation;
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f) * rotation;
    return glm::lookAt(position, center, up);
}

glm::mat4x4 camera::projection() const {
    return glm::perspective(glm::radians(75.f), 16.f / 9.f, 0.1f, 1000.f);
}