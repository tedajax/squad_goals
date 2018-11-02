#include "renderer.h"

#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <cstdio>

bool renderer::init(SDL_Window* window) {
    this->window = window;

    const char* glslVersion = "#version 410 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    gl = SDL_GL_CreateContext(window);

    // vsync
    SDL_GL_SetSwapInterval(1);

    if (gl3wInit() != GL3W_OK) {
        fprintf(stderr, "gl3w failed to init.\n");
        return false;
    }

    if (!gl3wIsSupported(4, 1)) {
        fprintf(stderr, "OpenGL 4.1 is not supported.\n");
        return false;
    }

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    glGenVertexArrays(2, &lineVertexArray);

    glBindVertexArray(lineVertexArray);

    glGenBuffers(3, &lineVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, lineColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(fillVertexArray);

    glGenBuffers(3, &fillVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, fillVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, fillColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);


    //glGenBuffers(3, &lineVertexBuffer);
    //glGenBuffers(3, &triVertexBuffer);
    
    

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

    glUseProgram(programId);
    mvpUniform = glGetUniformLocation(programId, "MVP");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

void renderer::shutdown() {
    SDL_GL_DeleteContext(gl);
}

void renderer::line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
    lineVertices.push_back(from);
    lineVertices.push_back(to);

    lineColors.push_back(glm::vec3(color));
    lineColors.push_back(glm::vec3(color));

    lineIndices.push_back(currentLineIndex++);
    lineIndices.push_back(currentLineIndex++);
}

void renderer::triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color) {
    line(a, b, color);
    line(b, c, color);
    line(c, a, color);
}

void renderer::fill_triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color) {
    fillVertices.push_back(a);
    fillVertices.push_back(b);
    fillVertices.push_back(c);

    fillColors.push_back(glm::vec3(color));
    fillColors.push_back(glm::vec3(color));
    fillColors.push_back(glm::vec3(color));

    fillIndices.push_back(currentFillIndex++);
    fillIndices.push_back(currentFillIndex++);
    fillIndices.push_back(currentFillIndex++);
}

void renderer::render(const camera& cam) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    auto model = glm::mat4(1.f);
    auto view = cam.view();
    auto projection = cam.projection();

    glm::mat4 mvp = projection * view * model;


    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);

    // triangles
    {
        glBindVertexArray(fillVertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, fillVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * fillVertices.size(), fillVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, fillColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * fillColors.size(), fillColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fillIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * fillIndices.size(), fillIndices.data(), GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, (GLsizei)fillIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    // lines
    {
        glBindVertexArray(lineVertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lineVertices.size(), lineVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, lineColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lineColors.size(), lineColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * lineIndices.size(), lineIndices.data(), GL_DYNAMIC_DRAW);

        glDrawElements(GL_LINES, (GLsizei)lineIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    lineVertices.clear();
    lineColors.clear();
    lineIndices.clear();
    currentLineIndex = 0;
    
    fillVertices.clear();
    fillColors.clear();
    fillIndices.clear();
    currentFillIndex = 0;
}