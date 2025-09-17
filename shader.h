#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::string readFile(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

GLuint LoadShaders(const char* vertexPath, const char* fragmentPath) {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vCode = readFile(vertexPath);
    std::string fCode = readFile(fragmentPath);

    const char* vSource = vCode.c_str();
    const char* fSource = fCode.c_str();

    glShaderSource(vShader, 1, &vSource, nullptr);
    glCompileShader(vShader);

    glShaderSource(fShader, 1, &fSource, nullptr);
    glCompileShader(fShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}

#endif