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
    void tri(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color);
    void render(const camera& cam);
private:
    SDL_GLContext gl;

    std::vector<glm::vec3> lineVertices;
    std::vector<glm::vec3> lineColors;
    std::vector<uint> lineIndices;

    std::vector<glm::vec3> triVertices;
    std::vector<glm::vec3> triColors;
    std::vector<uint> triIndices;

    uint currentLineIndex = 0;
    uint currentTriIndex = 0;

    GLuint lineVertexArray;
    GLuint triVertexArray;

    GLuint lineVertexBuffer;
    GLuint lineColorBuffer;
    GLuint lineIndexBuffer;

    GLuint triVertexBuffer;
    GLuint triColorBuffer;
    GLuint triIndexBuffer;

    GLuint programId;

    SDL_Window* window;
};