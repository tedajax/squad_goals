#pragma once

#include "types.h"
#include "camera.h"

#include <SDL2/SDL.h>

#include <vector>

class renderer {
public:
    bool init(SDL_Window* window);
    void shutdown();
    void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
    void triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
    void fill_triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
    void render(const camera& cam);

    const char* glslVersion() const { return "#version 410 core"; }
    SDL_GLContext  glContext() const { return gl; }

private:
    SDL_GLContext gl;

    std::vector<glm::vec3> lineVertices;
    std::vector<glm::vec3> lineColors;
    std::vector<uint> lineIndices;

    std::vector<glm::vec3> fillVertices;
    std::vector<glm::vec3> fillColors;
    std::vector<uint> fillIndices;

    uint currentLineIndex = 0;
    uint currentFillIndex = 0;

    uint lineVertexArray;
    uint fillVertexArray;

    uint lineVertexBuffer;
    uint lineColorBuffer;
    uint lineIndexBuffer;

    uint fillVertexBuffer;
    uint fillColorBuffer;
    uint fillIndexBuffer;

    uint programId;

    uint mvpUniform;

    SDL_Window* window;
};