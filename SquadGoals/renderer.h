#pragma once

#include "types.h"
#include "camera.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <vector>

class renderer {
public:
    bool init(SDL_Window* window);
    void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
    void render(const camera& cam);
private:
    SDL_GLContext gl;

    std::vector<f32> lineVertices;
    std::vector<f32> lineColors;
    std::vector<uint> lineIndices;

    uint currentIndex = 0;

    GLuint lineVertexBuffer;
    GLuint lineColorBuffer;
    GLuint lineIndexBuffer;

    GLuint programId;

    SDL_Window* window;
};