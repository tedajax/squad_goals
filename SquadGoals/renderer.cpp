#include "renderer.h"

#include <SDL2/SDL.h>
#include <cstdio>

bool renderer::init(SDL_Window* window) {
    this->window = window;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

    gl = SDL_GL_CreateContext(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
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

    glBindVertexArray(triVertexArray);

    glGenBuffers(3, &triVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, triColorBuffer);
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

void renderer::line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) {
    lineVertices.push_back(from);
    lineVertices.push_back(to);

    lineColors.push_back(glm::vec3(color));
    lineColors.push_back(glm::vec3(color));

    lineIndices.push_back(currentLineIndex++);
    lineIndices.push_back(currentLineIndex++);
}

void renderer::tri(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color) {
    triVertices.push_back(a);
    triVertices.push_back(b);
    triVertices.push_back(c);

    triColors.push_back(glm::vec3(color));
    triColors.push_back(glm::vec3(color));
    triColors.push_back(glm::vec3(color));

    triIndices.push_back(currentTriIndex++);
    triIndices.push_back(currentTriIndex++);
    triIndices.push_back(currentTriIndex++);
}

void renderer::render(const camera& cam) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    auto model = glm::mat4(1.f);
    auto view = cam.look_at();
    auto projection = cam.projection();

    glm::mat4 mvp = projection * view * model;


    GLuint mvpUniform = glGetUniformLocation(programId, "MVP");
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, &mvp[0][0]);

    // triangles
    {
        glBindVertexArray(triVertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * triVertices.size(), triVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, triColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * triColors.size(), triColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * triIndices.size(), triIndices.data(), GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, triIndices.size(), GL_UNSIGNED_INT, nullptr);
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

        glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    SDL_GL_SwapWindow(window);

    lineVertices.clear();
    lineColors.clear();
    lineIndices.clear();
    currentLineIndex = 0;
    
    triVertices.clear();
    triColors.clear();
    triIndices.clear();
    currentTriIndex = 0;
}