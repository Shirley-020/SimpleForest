#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

static std::string readFile(const char* path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Failed to open: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

Shader::Shader() : ID(0) {}

bool Shader::load(const char* vertPath, const char* fragPath) {
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    const char* v = vertSrc.c_str();
    const char* f = fragSrc.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &v, NULL);
    glCompileShader(vs);
    if (!checkCompile(vs, "VERTEX")) return false;

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &f, NULL);
    glCompileShader(fs);
    if (!checkCompile(fs, "FRAGMENT")) return false;

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024]; glGetProgramInfoLog(ID, 1024, NULL, info);
        std::cerr << "PROGRAM LINK ERROR:\n" << info << std::endl;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setMat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name), x, y, z);
}

void Shader::setVec3(const char* name, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(v));
}

void Shader::setFloat(const char* name, float f) const {
    glUniform1f(glGetUniformLocation(ID, name), f);
}

void Shader::setInt(const char* name, int v) const {
    glUniform1i(glGetUniformLocation(ID, name), v);
}

void Shader::setBool(const char* name, bool v) const {
    glUniform1i(glGetUniformLocation(ID, name), (int)v);
}

bool Shader::checkCompile(unsigned int shader, const char* type) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, NULL, info);
        std::cerr << type << " SHADER COMPILE ERROR:\n" << info << std::endl;
        return false;
    }
    return true;
}

