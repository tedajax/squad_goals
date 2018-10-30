#include "renderer.h"

#include <SDL2/SDL.h>
#include <cstdio>

bool renderer::init(SDL_Window* window) {
    this->window = window;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    gl = SDL_GL_CreateContext(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        return false;
    }

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    glGenBuffers(1, &lineVertexBuffer);
    glGenBuffers(1, &lineColorBuffer);
    glGenBuffers(1, &lineIndexBuffer);
    
    const char* vertexShader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 vertexPosition;\n"
        "layout(location = 1) in vec3 vertexColor;\n"
        "out vec3 fragmentColor;\n"
        "uniform mat4 MVP;\n"
        "void main(){\n"
        " gl_Position = MVP * vec4(vertexPosition, 1.0);\n"
        " fragmentColor = vertexColor;\n"
        "}";

    const char* fragmentShader =
        "#version 330 core\n"
        "in vec3 fragmentColor;\n"
        "out vec3 color;\n"
        "void main(){\n"
        " color = fragmentColor;\n"
        "}";

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShaderId, 1, &vertexShader, nullptr);
    glCompileShader(vertexShaderId);

    glShaderSource(fragmentShaderId, 1, &fragmentShader, nullptr);
    glCompileShader(fragmentShaderId);

    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);

    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return true;
}

void renderer::line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
    lineVertices.push_back(from.x);
    lineVertices.push_back(from.y);
    lineVertices.push_back(from.z);

    lineVertices.push_back(to.x);
    lineVertices.push_back(to.y);
    lineVertices.push_back(to.z);

    lineColors.push_back(color.r);
    lineColors.push_back(color.g);
    lineColors.push_back(color.b);

    lineColors.push_back(color.r);
    lineColors.push_back(color.g);
    lineColors.push_back(color.b);

    lineIndices.push_back(currentIndex++);
    lineIndices.push_back(currentIndex++);
}

void renderer::render(const camera& cam) {
    glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * lineVertices.size(), &lineVertices[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, lineColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * lineColors.size(), &lineColors[0], GL_DYNAMIC_DRAW);

    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * lineIndices.size(), &lineVertices[0], GL_DYNAMIC_DRAW);
*/

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    auto model = glm::mat4(1.f);
    auto view = cam.look_at();
    auto projection = cam.projection();

    glm::mat4 mvp = projection * view * model;

    GLuint mvpUniform = glGetUniformLocation(programId, "MVP");
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, lineColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndexBuffer);
    //glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    SDL_GL_SwapWindow(window);

    lineVertices.clear();
    lineColors.clear();
    lineIndices.clear();
    currentIndex = 0;
}