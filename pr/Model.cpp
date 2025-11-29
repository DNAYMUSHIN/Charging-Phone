// Model.cpp
#include "model.h"
#include "func.h"

GLuint compile_shader(const char* src, GLenum type) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetShaderInfoLog(s, len, nullptr, &log[0]);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return s;
}

void Model::load_shaders(const char* vect, const char* frag) {
    std::string vs_src = LoadShader(vect);
    std::string fs_src = LoadShader(frag);
    const char* vs_c = vs_src.c_str();
    const char* fs_c = fs_src.c_str();

    GLuint vs = compile_shader(vs_c, GL_VERTEX_SHADER);
    GLuint fs = compile_shader(fs_c, GL_FRAGMENT_SHADER);

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    GLint ok;
    glGetProgramiv(shader_programme, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len;
        glGetProgramiv(shader_programme, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetProgramInfoLog(shader_programme, len, nullptr, &log[0]);
        std::cerr << "Program link error: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Model::load_coords(glm::vec3* verteces, size_t count) {
    verteces_count = count;
    glBindVertexArray(vao);

    if (vbo_coords == 0) glGenBuffers(1, &vbo_coords);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), verteces, GL_STATIC_DRAW);

    // vertex attrib setup will be done in render (when program is used)
    glBindVertexArray(0);
}

void Model::load_colors(glm::vec3* colors, size_t count) {
    glBindVertexArray(vao);
    if (vbo_colors == 0) glGenBuffers(1, &vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), colors, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Model::load_indices(GLuint* indices, size_t count) {
    indices_count = count;
    glBindVertexArray(vao);
    if (ibo == 0) glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Model::render(GLuint mode) {
    glBindVertexArray(vao);

    // перед рендером надо назначить атрибуты (если есть программа)
    GLint posLoc = 0;
    GLint colLoc = 1;

    if (vbo_coords) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    if (vbo_colors) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
        glEnableVertexAttribArray(colLoc);
        glVertexAttribPointer(colLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    if (ibo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(mode, (GLsizei)indices_count, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(mode, 0, (GLsizei)verteces_count);
    }

    // выключим атрибуты
    if (vbo_coords) glDisableVertexAttribArray(posLoc);
    if (vbo_colors) glDisableVertexAttribArray(colLoc);

    glBindVertexArray(0);
}

// дополнительные приватные поля:
