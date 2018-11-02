#include "camera.h"
#include <cstdio>

void camera::move(glm::vec3 movement) {
    position += glm::vec3(movement.x, 0, movement.z);
}

void camera::move_relative(glm::vec3 movement) {
    glm::vec4 tl = glm::rotate(glm::mat4(1.f), glm::radians(180 - theta), glm::vec3(0, 1, 0)) * glm::vec4(-movement, 1);
    glm::vec3 mv = glm::vec3(tl.x, 0, tl.z);

    position += mv;
}

void camera::orbit(f32 pitch, f32 yaw) {
    phi += pitch;
    theta += yaw;

    phi = math::clamp(phi, 0.1f, 89.9f);
}

glm::mat4x4 camera::view() const {
    glm::mat4 v = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(1, 0, 0));
    v = glm::rotate(v, glm::radians(0.f), glm::vec3(0, 1, 0));
    v = glm::translate(v, position + glm::vec3(math::sin(phi) * math::cos(theta) * rho, math::cos(phi) * rho, math::sin(phi) * math::sin(theta) * rho));
    v = glm::inverse(v);


    glm::vec3 pos = position + glm::vec3(math::sin(phi) * math::cos(theta + 90.f) * rho, math::cos(phi) * rho, math::sin(phi) * math::sin(theta + 90.f) * rho);
    auto la = glm::lookAt(pos, position, glm::vec3(0, 1, 0));
    return la;
}

glm::mat4x4 camera::projection() const {
    return glm::perspective(glm::radians(75.f), 16.f / 9.f, 0.1f, 1000.f);
}