#include "camera.h"
#include <cstdio>

void camera::move(glm::vec3 movement) {
    position += movement;
}

void camera::move_relative(glm::vec3 movement) {
    position += rotation * movement;
}

void camera::rotate(glm::quat rotation) {
    this->rotation = rotation * this->rotation;
}

glm::mat4x4 camera::look_at() const {
    auto forwardWorld = rotation * glm::vec3(0.f, 0.f, -1.f);
    auto upWorld = rotation * glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 center = position + forwardWorld;
    return glm::lookAt(position, center, upWorld);
}

glm::mat4x4 camera::projection() const {
    return glm::perspective(glm::radians(75.f), 16.f / 9.f, 0.1f, 1000.f);
}