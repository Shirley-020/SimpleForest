#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../SimpleForest/include/stb/stb_image.h"  //改成绝对路径

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// 新增 Color 结构体定义
struct Color {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

// 材质定义
Color woodColor = { glm::vec3(0.4f, 0.3f, 0.2f), glm::vec3(0.7f, 0.6f, 0.5f), glm::vec3(0.1f), 32.0f };
Color roofColor = { glm::vec3(0.3f, 0.1f, 0.1f), glm::vec3(0.6f, 0.2f, 0.1f), glm::vec3(0.1f), 16.0f };
Color windowColor = { glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.2f, 0.2f, 0.4f), glm::vec3(0.8f), 128.0f };
Color doorColor = { glm::vec3(0.3f, 0.2f, 0.1f), glm::vec3(0.5f, 0.4f, 0.2f), glm::vec3(0.05f), 32.0f };
Color treeTrunkColor = { glm::vec3(0.3f, 0.2f, 0.1f), glm::vec3(0.5f, 0.4f, 0.2f), glm::vec3(0.05f), 8.0f };
Color treeCrownColor = { glm::vec3(0.1f, 0.3f, 0.1f), glm::vec3(0.2f, 0.6f, 0.2f), glm::vec3(0.1f), 32.0f };
Color chimneyColor = { glm::vec3(0.2f), glm::vec3(0.4f), glm::vec3(0.0f), 4.0f };
Color stepColor = { glm::vec3(0.3f), glm::vec3(0.5f), glm::vec3(0.0f), 4.0f };

// -------------------- Shader 类 --------------------
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

class Shader {
public:
    unsigned int ID;
    Shader() : ID(0) {}
    bool load(const char* vertPath, const char* fragPath) {
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

    void use() const { glUseProgram(ID); }
    void setMat4(const char* name, const glm::mat4& m) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(m));
    }
    void setVec3(const char* name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }
    void setVec3(const char* name, const glm::vec3& v) const {
        glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(v));
    }
    void setFloat(const char* name, float f) const {
        glUniform1f(glGetUniformLocation(ID, name), f);
    }
    void setInt(const char* name, int v) const {
        glUniform1i(glGetUniformLocation(ID, name), v);
    }
    void setBool(const char* name, bool v) const {
        glUniform1i(glGetUniformLocation(ID, name), (int)v);
    }
private:
    bool checkCompile(unsigned int shader, const char* type) {
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
};

// -------------------- Camera --------------------
struct Camera {
    glm::vec3 pos{ 0.0f, 1.6f, 5.0f };
    glm::vec3 front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    float yaw = -90.f;
    float pitch = 0.f;
    float speed = 3.5f;
    float sensitivity = 0.1f;
    bool firstMouse = true;
    float lastX = 400, lastY = 300;

    glm::mat4 getView() const {
        return glm::lookAt(pos, pos + front, up);
    }

    void processMouse(float xpos, float ypos) {
        if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos; lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;
        yaw += xoffset;
        pitch += yoffset;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(f);
    }

    void processKeyboard(int direction, float deltaTime) {
        float velocity = speed * deltaTime;
        if (direction == 0) pos += front * velocity;
        if (direction == 1) pos -= front * velocity;
        if (direction == 2) pos -= glm::normalize(glm::cross(front, up)) * velocity;
        if (direction == 3) pos += glm::normalize(glm::cross(front, up)) * velocity;
    }
};

// -------------------- Mesh --------------------
struct Mesh {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    GLsizei indexCount = 0;
};

Mesh createCube() {
    float v[] = {
        // 位置              // 法线              // 纹理坐标
        // Front face
        -0.5f,-0.5f, 0.5f,  0.0f,0.0f,1.0f,      0.0f, 0.0f, // 0
         0.5f,-0.5f, 0.5f,  0.0f,0.0f,1.0f,      1.0f, 0.0f, // 1
         0.5f, 0.5f, 0.5f,  0.0f,0.0f,1.0f,      1.0f, 1.0f, // 2
        -0.5f, 0.5f, 0.5f,  0.0f,0.0f,1.0f,      0.0f, 1.0f, // 3

        // Back face
        -0.5f,-0.5f,-0.5f,  0.0f,0.0f,-1.0f,     1.0f, 0.0f, // 4 (注意 UV 翻转)
         0.5f,-0.5f,-0.5f,  0.0f,0.0f,-1.0f,     0.0f, 0.0f, // 5
         0.5f, 0.5f,-0.5f,  0.0f,0.0f,-1.0f,     0.0f, 1.0f, // 6
        -0.5f, 0.5f,-0.5f,  0.0f,0.0f,-1.0f,     1.0f, 1.0f, // 7

        // Left face
        -0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,     0.0f, 0.0f, // 8
        -0.5f,-0.5f, 0.5f,  -1.0f,0.0f,0.0f,     1.0f, 0.0f, // 9
        -0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,     1.0f, 1.0f, // 10
        -0.5f, 0.5f,-0.5f,  -1.0f,0.0f,0.0f,     0.0f, 1.0f, // 11

        // Right face
         0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,     1.0f, 0.0f, // 12
         0.5f,-0.5f, 0.5f,   1.0f,0.0f,0.0f,     0.0f, 0.0f, // 13
         0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,     0.0f, 1.0f, // 14
         0.5f, 0.5f,-0.5f,   1.0f,0.0f,0.0f,     1.0f, 1.0f, // 15

         // Top face
         -0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,     0.0f, 0.0f, // 16
          0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,     1.0f, 0.0f, // 17
          0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,     1.0f, 1.0f, // 18
         -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,     0.0f, 1.0f, // 19

         // Bottom face
         -0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,    0.0f, 0.0f, // 20
          0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,    1.0f, 0.0f, // 21
          0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,    1.0f, 1.0f, // 22
         -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,    0.0f, 1.0f  // 23
    };

    unsigned int indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Pos
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(2); // 纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // TexCoord

    glBindVertexArray(0);
    m.indexCount = sizeof(indices) / sizeof(indices[0]);
    return m;
}

Mesh createCone(int segments = 20, float height = 1.0f, float radius = 0.8f) {
    Mesh m;
    std::vector<float> verts;
    std::vector<unsigned int> indices;

    // 每顶点 8 个浮点数：Position(3) + Normal(3) + UV(2)
    const int stride = 8;
    int vertexCount = 0;

    // 1. 顶部顶点 (Tip)
    // 树冠的尖端
    verts.push_back(0.0f); verts.push_back(height); verts.push_back(0.0f); // Pos
    verts.push_back(0.0f); verts.push_back(1.0f); verts.push_back(0.0f);  // Normal (这里为了简化，使用向上法线)
    verts.push_back(0.5f); verts.push_back(1.0f);                        // UV (V=1.0 代表顶部)
    vertexCount++;

    // 2. 侧面顶点 (Sides)
    float slantHeight = sqrt(radius * radius + height * height);

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * M_PI;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // 位置 (Pos)
        verts.push_back(x); verts.push_back(0.0f); verts.push_back(z);

        // 法线 (Normal) - 用于平滑着色，指向斜面外侧
        glm::vec3 n = glm::normalize(glm::vec3(x, 0.0f, z) * (slantHeight / radius) + glm::vec3(0.0f, height / radius, 0.0f));
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);

        // 纹理坐标 (UV)
        float u_coord = (float)i / segments; // U 坐标环绕圆锥体 (0 到 1)
        verts.push_back(u_coord); verts.push_back(0.0f); // V 坐标为 0.0 代表底部

        vertexCount++;
    }

    // 侧面索引 (三角形扇)
    for (int i = 0; i < segments; i++) {
        // 顶部顶点 (0) -> 当前底部顶点 (i+1) -> 下一个底部顶点 (i+2)
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // 3. 底部顶点和索引 (Base) - 如果需要贴图，则需要为底面单独计算
    // 此处为了简化树冠的绘制，我们跳过底面贴图，因为它通常不被看到。
    // 如果需要底面，则需要创建中心点和底面边缘点的重复顶点，并计算扇形 UVs。

    m.indexCount = indices.size();

    // -------------------- VAO 配置 --------------------
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);

    // Normal (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));

    // TexCoord (2 floats) 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    return m;
}

Mesh createCylinder(int segments = 16, float height = 1.0f, float radius = 0.2f) {
    std::vector<float> verts;
    std::vector<unsigned int> inds;

    // 侧面顶点和法线
    for (int j = 0; j < 2; j++) {
        float y = (j == 0) ? 0.0f : height;
        float v_coord = (j == 0) ? 0.0f : 1.0f; // V坐标：底部0，顶部1
        for (int i = 0; i < segments; i++) {
            float a = (float)i / segments * 2.0f * M_PI;
            float x = cos(a) * radius;
            float z = sin(a) * radius;

            verts.push_back(x); verts.push_back(y); verts.push_back(z); // Position

            // 法线：计算水平法线，y分量为0
            glm::vec3 n = glm::normalize(glm::vec3(x, 0.0f, z));
            verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // Normal 

            float u_coord = (float)i / segments; // U坐标：0到1
            verts.push_back(u_coord); verts.push_back(v_coord); // UVs
        }
    }

    // 索引部分
    for (int i = 0; i < segments; i++) {
        int a = i;
        int b = (i + 1) % segments;
        int a2 = a + segments;
        int b2 = b + segments;
        inds.push_back(a); inds.push_back(b); inds.push_back(b2);
        inds.push_back(b2); inds.push_back(a2); inds.push_back(a);
    }

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Pos
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // TexCoord

    glBindVertexArray(0);
    m.indexCount = (GLsizei)inds.size();
    return m;
}

Mesh createWindow(float width = 0.3f, float height = 0.4f) {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    // 每个顶点：Pos(3) + Normal(3) + TexCoord(2) = 8 floats
    float vertices[] = {
        // ---- 外框 ----
        -halfW, -halfH, 0.0f,   0.0f,0.0f,1.0f,   0.0f,0.0f,
         halfW, -halfH, 0.0f,   0.0f,0.0f,1.0f,   1.0f,0.0f,
         halfW,  halfH, 0.0f,   0.0f,0.0f,1.0f,   1.0f,1.0f,
        -halfW,  halfH, 0.0f,   0.0f,0.0f,1.0f,   0.0f,1.0f,

        // ---- 内层玻璃 ----
        -halfW * 0.8f, -halfH * 0.8f, 0.01f,   0.0f,0.0f,1.0f,   0.0f,0.0f,
         halfW * 0.8f, -halfH * 0.8f, 0.01f,   0.0f,0.0f,1.0f,   1.0f,0.0f,
         halfW * 0.8f,  halfH * 0.8f, 0.01f,   0.0f,0.0f,1.0f,   1.0f,1.0f,
        -halfW * 0.8f,  halfH * 0.8f, 0.01f,   0.0f,0.0f,1.0f,   0.0f,1.0f
    };

    unsigned int indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // TexCoord（新增）
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    m.indexCount = 12;
    return m;
}


Mesh createDoor(float width = 0.5f, float height = 1.0f) {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    // 每个顶点：Pos(3) + Normal(3) + TexCoord(2) = 8 floats
    float vertices[] = {
        // ---- 外框 ----
        -halfW, -halfH, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         halfW, -halfH, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         halfW,  halfH, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -halfW,  halfH, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

        // ---- 内层门板 ----
        -halfW * 0.9f, -halfH * 0.9f, 0.01f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f,
         halfW * 0.9f, -halfH * 0.9f, 0.01f,   0.0f, 0.0f, 1.0f,   0.9f, 0.1f,
         halfW * 0.9f,  halfH * 0.9f, 0.01f,   0.0f, 0.0f, 1.0f,   0.9f, 0.9f,
        -halfW * 0.9f,  halfH * 0.9f, 0.01f,   0.0f, 0.0f, 1.0f,   0.1f, 0.9f
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,    // 外框
        4, 5, 6,  6, 7, 4     // 内层门板
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // TexCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    m.indexCount = sizeof(indices) / sizeof(indices[0]);
    return m;
}


Mesh createRoof(float width = 4.0f, float depth = 5.0f, float pitch = 0.5f) {
    float halfW = width * 0.5f;
    float overhang = 0.0f; // 屋檐伸出墙体的距离
    float roofHeight = width * pitch * 0.5f; // 基于宽度计算高度，pitch 是坡度

    std::vector<float> vertices = {
        // ------------------ 原始屋顶的 12 个顶点 ------------------
        // P(3), N(3), T(2) -> 8 floats per vertex

        // 0: 前屋檐左下 (-halfW, 0.0f, overhang)
        -halfW, 0.0f, overhang,     0.0f, 0.447f, 0.894f,    0.0f, 0.0f,
        // 1: 前屋檐右下 (halfW, 0.0f, overhang)
         halfW, 0.0f, overhang,     0.0f, 0.447f, 0.894f,    1.0f, 0.0f,
         // 2: 前屋顶尖 (0.0f, roofHeight, 0.0f)
          0.0f, roofHeight, 0.0f,    0.0f, 0.447f, 0.894f,    0.5f, 1.0f,

          // 3: 后屋檐左下 (-halfW, 0.0f, -depth)
          -halfW, 0.0f, -depth,       0.0f, 0.447f, -0.894f,   0.0f, 0.0f,
          // 4: 后屋檐右下 (halfW, 0.0f, -depth)
           halfW, 0.0f, -depth,       0.0f, 0.447f, -0.894f,   1.0f, 0.0f,
           // 5: 后屋顶尖 (0.0f, roofHeight, -depth)
            0.0f, roofHeight, -depth,  0.0f, 0.447f, -0.894f,   0.5f, 1.0f,

            // 6: 左侧面左下 (-halfW, 0.0f, 0.0f)
            -halfW, 0.0f, 0.0f,         -0.894f, 0.447f, 0.0f,   0.0f, 0.0f,
            // 7: 左侧面右下 (-halfW, 0.0f, -depth)
            -halfW, 0.0f, -depth,       -0.894f, 0.447f, 0.0f,   1.0f, 0.0f,
            // 8: 左侧面中上 (0.0f, roofHeight, -depth/2)
             0.0f, roofHeight, -depth / 2, -0.894f, 0.447f, 0.0f, 0.5f, 1.0f,

             // 9: 右侧面左下 (halfW, 0.0f, 0.0f)
              halfW, 0.0f, 0.0f,         0.894f, 0.447f, 0.0f,    0.0f, 0.0f,
              // 10: 右侧面右下 (halfW, 0.0f, -depth)
               halfW, 0.0f, -depth,       0.894f, 0.447f, 0.0f,    1.0f, 0.0f,
               // 11: 右侧面中上 (0.0f, roofHeight, -depth/2)
                0.0f, roofHeight, -depth / 2, 0.894f, 0.447f, 0.0f,  0.5f, 1.0f,

                // ------------------ 新增的 4 个底面顶点 (索引从 12 开始) ------------------
                // 法线朝下: (0.0, -1.0, 0.0)

                // 12: 底面 - 前左 (-halfW, 0.0f, overhang)
                -halfW, 0.0f, overhang,     0.0f, -1.0f, 0.0f,    0.0f, 0.0f,
                // 13: 底面 - 前右 (halfW, 0.0f, overhang)
                 halfW, 0.0f, overhang,     0.0f, -1.0f, 0.0f,    1.0f, 0.0f,
                 // 14: 底面 - 后右 (halfW, 0.0f, -depth)
                  halfW, 0.0f, -depth,       0.0f, -1.0f, 0.0f,    1.0f, 1.0f,
                  // 15: 底面 - 后左 (-halfW, 0.0f, -depth)
                  -halfW, 0.0f, -depth,       0.0f, -1.0f, 0.0f,    0.0f, 1.0f
    };

    // 索引数据
    std::vector<unsigned int> indices = {
        // 原始屋顶的四个面
        0, 1, 2,     // 前三角形
        3, 4, 5,     // 后三角形
        6, 7, 8,     // 左侧面 
        9, 10, 11,   // 右侧面 
        12, 13, 14,  // 底面三角形 1
        14, 15, 12   // 底面三角形 2 
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);

    // 顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 顶点属性指针 (Stride = 8 * sizeof(float))
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // 纹理坐标属性
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    m.indexCount = indices.size();
    return m;
}


Mesh createSkybox() {
    float scale = 100.0f;
    float skyboxVertices[] = {
        -1.0f * scale,  1.0f * scale, -1.0f * scale,
        -1.0f * scale, -1.0f * scale, -1.0f * scale,
         1.0f * scale, -1.0f * scale, -1.0f * scale,
         1.0f * scale, -1.0f * scale, -1.0f * scale,
         1.0f * scale,  1.0f * scale, -1.0f * scale,
        -1.0f * scale,  1.0f * scale, -1.0f * scale,

        -1.0f * scale, -1.0f * scale,  1.0f * scale,
        -1.0f * scale, -1.0f * scale, -1.0f * scale,
        -1.0f * scale,  1.0f * scale, -1.0f * scale,
        -1.0f * scale,  1.0f * scale, -1.0f * scale,
        -1.0f * scale,  1.0f * scale,  1.0f * scale,
        -1.0f * scale, -1.0f * scale,  1.0f * scale,

         1.0f * scale, -1.0f * scale, -1.0f * scale,
         1.0f * scale, -1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale, -1.0f * scale,
         1.0f * scale, -1.0f * scale, -1.0f * scale,

        -1.0f * scale, -1.0f * scale,  1.0f * scale,
        -1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale, -1.0f * scale,  1.0f * scale,
        -1.0f * scale, -1.0f * scale,  1.0f * scale,

        -1.0f * scale,  1.0f * scale, -1.0f * scale,
         1.0f * scale,  1.0f * scale, -1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
         1.0f * scale,  1.0f * scale,  1.0f * scale,
        -1.0f * scale,  1.0f * scale,  1.0f * scale,
        -1.0f * scale,  1.0f * scale, -1.0f * scale,

        -1.0f * scale, -1.0f * scale, -1.0f * scale,
        -1.0f * scale, -1.0f * scale,  1.0f * scale,
         1.0f * scale, -1.0f * scale, -1.0f * scale,
         1.0f * scale, -1.0f * scale, -1.0f * scale,
        -1.0f * scale, -1.0f * scale,  1.0f * scale,
         1.0f * scale, -1.0f * scale,  1.0f * scale
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    m.indexCount = 36;
    return m;
}

// -------------------- 纹理加载函数 --------------------
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(false); // 纹理通常不需要翻转，但有些贴图需要
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理包裹和过滤参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << path << std::endl;
    }
    else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// 渲染优化的小屋
void renderDetailedHouse(Shader& shader, Mesh& cube, Mesh& roof, Mesh& windowMesh, Mesh& doorMesh,
    bool useTexture, unsigned int woodTex, unsigned int roofTex, unsigned int stepTex, 
    unsigned int windowTex, unsigned int doorTex)
{
    // 在每个物体渲染前都要设置纹理采样器
    shader.setInt("texture_diffuse1", 0);
    
    // -------------------- 1. 小屋主体 --------------------
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(3.0f, 2.0f, 4.0f));
    shader.setMat4("model", model);

    shader.setBool("useTexture", useTexture);
    shader.setVec3("material.ambient", woodColor.ambient);
    shader.setVec3("material.specular", woodColor.specular);
    shader.setFloat("material.shininess", woodColor.shininess);

    // 绑定木纹纹理
    glActiveTexture(GL_TEXTURE0);
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, woodTex);
        shader.setVec3("material.diffuse", glm::vec3(1.0f));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0); // 绑定空纹理
        shader.setVec3("material.diffuse", woodColor.diffuse);
    }

    glBindVertexArray(cube.VAO);
    glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

    // -------------------- 2. 屋顶 --------------------
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 2.45f, 0.0f));
    shader.setMat4("model", model);

    // 绑定屋顶纹理
    glActiveTexture(GL_TEXTURE0);
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, roofTex);
        shader.setBool("useTexture", true);
        shader.setVec3("material.diffuse", glm::vec3(1.0f));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setBool("useTexture", false);
        shader.setVec3("material.diffuse", roofColor.diffuse);
    }

    shader.setVec3("material.ambient", roofColor.ambient);
    shader.setVec3("material.specular", roofColor.specular);
    shader.setFloat("material.shininess", roofColor.shininess);

    glBindVertexArray(roof.VAO);
    glDrawElements(GL_TRIANGLES, roof.indexCount, GL_UNSIGNED_INT, 0);

    // -------------------- 3. 烟囱 --------------------
    model = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 3.5f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f, 1.0f, 0.4f));
    shader.setMat4("model", model);

    shader.setBool("useTexture", false); // 烟囱强制纯色
    glBindTexture(GL_TEXTURE_2D, 0); // 确保没有绑定纹理

    shader.setVec3("material.ambient", chimneyColor.ambient);
    shader.setVec3("material.diffuse", chimneyColor.diffuse);
    shader.setVec3("material.specular", chimneyColor.specular);
    shader.setFloat("material.shininess", chimneyColor.shininess);
    
    glBindVertexArray(cube.VAO);
    glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

    // -------------------- 4. 前门 --------------------
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.5f, 2.01f)); // 稍微突出墙面
    // 移除旋转，让门正对相机
    shader.setMat4("model", model);

    // 绑定门纹理
    glActiveTexture(GL_TEXTURE0);
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, doorTex);
        shader.setBool("useTexture", true);
        shader.setVec3("material.diffuse", glm::vec3(1.0f));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setBool("useTexture", false);
        shader.setVec3("material.diffuse", doorColor.diffuse);
    }

    shader.setVec3("material.ambient", doorColor.ambient);
    shader.setVec3("material.specular", doorColor.specular);
    shader.setFloat("material.shininess", doorColor.shininess);

    glBindVertexArray(doorMesh.VAO);
    glDrawElements(GL_TRIANGLES, doorMesh.indexCount, GL_UNSIGNED_INT, 0);

    // -------------------- 5. 窗户 --------------------
    // 绑定窗户纹理
    glActiveTexture(GL_TEXTURE0);
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, windowTex);
        shader.setBool("useTexture", true);
        shader.setVec3("material.diffuse", glm::vec3(1.0f));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setBool("useTexture", false);
        shader.setVec3("material.diffuse", windowColor.diffuse);
    }

    shader.setVec3("material.ambient", windowColor.ambient);
    shader.setVec3("material.specular", windowColor.specular);
    shader.setFloat("material.shininess", windowColor.shininess);

    // 窗户 1
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.2f, 2.01f));
    shader.setMat4("model", model);
    glBindVertexArray(windowMesh.VAO);
    glDrawElements(GL_TRIANGLES, windowMesh.indexCount, GL_UNSIGNED_INT, 0);

    // 窗户 2
    model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.2f, 2.01f));
    shader.setMat4("model", model);
    glBindVertexArray(windowMesh.VAO);
    glDrawElements(GL_TRIANGLES, windowMesh.indexCount, GL_UNSIGNED_INT, 0);

    // -------------------- 6. 台阶 --------------------
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 2.5f));
    model = glm::scale(model, glm::vec3(1.0f, 0.1f, 1.0f));
    shader.setMat4("model", model);

    // 绑定台阶纹理
    glActiveTexture(GL_TEXTURE0);
    if (useTexture) {
        glBindTexture(GL_TEXTURE_2D, stepTex);
        shader.setBool("useTexture", true);
        shader.setVec3("material.diffuse", glm::vec3(1.0f));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setBool("useTexture", false);
        shader.setVec3("material.diffuse", stepColor.diffuse);
    }

    shader.setVec3("material.ambient", stepColor.ambient);
    shader.setVec3("material.specular", stepColor.specular);
    shader.setFloat("material.shininess", stepColor.shininess);

    glBindVertexArray(cube.VAO);
    glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);
}

// 定义树木结构体，包含位置和随机缩放因子
struct Tree {
    glm::vec3 position;
    float scale; // 随机缩放因子
};

// 修改后的随机树木生成函数
std::vector<Tree> generateRandomTrees(
    int treeCount, float xMin = -12.0f, float xMax = 12.0f,
    float zMin = -20.0f, float zMax = 20.0f,
    float houseXRange = 4.0f, float houseZRange = 4.0f,
    float minDistance = 2.0f) {

    std::vector<Tree> trees;

    while (trees.size() < treeCount) {
        float randX = xMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (xMax - xMin)));
        float randZ = zMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (zMax - zMin)));

        // 避开房子区域
        if (abs(randX) < houseXRange && abs(randZ) < houseZRange) {
            continue;
        }

        // 检查与其他树木的距离
        bool isTooClose = false;
        for (const auto& tree : trees) {
            float distance = glm::distance(glm::vec2(randX, randZ), glm::vec2(tree.position.x, tree.position.z));
            if (distance < minDistance) {
                isTooClose = true;
                break;
            }
        }

        if (!isTooClose) {
            // 生成随机缩放因子 (0.7 到 1.5 之间)
            float randomScale = 0.7f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)) * 0.8f;
            trees.push_back({ glm::vec3(randX, 0.0f, randZ), randomScale });
        }
    }
    return trees;
}


// 全局变量
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
Camera camera;
bool keys[1024];
float deltaTime = 0.0f, lastFrame = 0.0f;
bool captureMouse = true;
bool useTextureGlobally = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width; SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (captureMouse) camera.processMouse((float)xpos, (float)ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // 添加Tab键切换鼠标捕获
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        captureMouse = !captureMouse;
        glfwSetInputMode(window, GLFW_CURSOR, captureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (captureMouse) {
            // 鼠标捕获后设置到窗口中心
            glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            camera.firstMouse = true;
        }
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void processInput(float dt) {
    if (keys[GLFW_KEY_W]) camera.processKeyboard(0, dt);
    if (keys[GLFW_KEY_S]) camera.processKeyboard(1, dt);
    if (keys[GLFW_KEY_A]) camera.processKeyboard(2, dt);
    if (keys[GLFW_KEY_D]) camera.processKeyboard(3, dt);
}

int main() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n"; return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleForest", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n"; glfwTerminate(); return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n"; return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
    // 初始化随机数种子（每棵树的数量/大小/高度随机）
    srand((unsigned int)glfwGetTime());

    glEnable(GL_DEPTH_TEST);

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
	
    // 加载着色器
    Shader basicShader;
    if (!basicShader.load("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Failed to load basic shaders\n";
        return -1;
    }

    Shader skyboxShader;
    if (!skyboxShader.load("shaders/skybox.vs", "shaders/skybox.fs")) {
        std::cerr << "Failed to load skybox shaders\n";
        return -1;
    }

    // 创建网格
    Mesh cube = createCube();
    Mesh roof = createRoof(8.0f, 3.0f, 0.5f);
    Mesh cylinder = createCylinder(20, 1.2f, 0.15f);
    Mesh windowMesh = createWindow();
    Mesh doorMesh = createDoor();
    Mesh skybox = createSkybox();
    Mesh cone = createCone(28, 1.6f, 1.0f);

    // 加载纹理
    std::vector<std::string> skyboxFaces = {
        "../SimpleForest/objects/right.jpg",
        "../SimpleForest/objects/left.jpg",
        "../SimpleForest/objects/top.jpg",
        "../SimpleForest/objects/bottom.jpg",
        "../SimpleForest/objects/front.jpg",
        "../SimpleForest/objects/back.jpg"
    };
    unsigned int skyboxTexture = loadCubemap(skyboxFaces);

    // 加载新的纹理文件
	unsigned int woodTexture = loadTexture("../SimpleForest/objects/wood_texture.jpg");   // 木纹纹理
	unsigned int barkTexture = loadTexture("../SimpleForest/objects/bark_texture.jpg");    //  树干纹理
	unsigned int leavesTexture = loadTexture("../SimpleForest/objects/leaves_texture.jpg");   // 树叶纹理
    unsigned int roofTexture = loadTexture("../SimpleForest/objects/roof_shingles.jpg");        // 屋顶瓦片纹理
    unsigned int stepTexture = loadTexture("../SimpleForest/objects/stone_step.jpg");          // 石头台阶纹理
    unsigned int windowGlassTexture = loadTexture("../SimpleForest/objects/window_glass.jpg"); // 玻璃纹理 
	unsigned int doorTexture = loadTexture("../SimpleForest/objects/door.jpg");              // 木门纹理
    // 光照参数
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f));
    glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);

    // 投影矩阵
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 200.0f);


    // 生成随机树木位置
    std::vector<Tree> trees = generateRandomTrees(
        50, -14.0f, 14.0f, -17.0f, 20.0f, 4.0f, 4.0f, 2.0f);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        float current = (float)glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        processInput(deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制场景物体
        basicShader.use();
        // 修复 setVec3 参数为 glm::vec3 对象
        basicShader.setVec3("lightDir", lightDir);
        basicShader.setVec3("lightColor", lightColor);
        basicShader.setVec3("viewPos", camera.pos);
        basicShader.setMat4("proj", proj);
        basicShader.setMat4("view", camera.getView());

        // -------------------- 绘制小屋 --------------------
        // 绑定木纹纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        basicShader.setInt("texture_diffuse1", 0);

        // 调用修改后的函数
        // 在 main 函数中，修改调用方式：
        renderDetailedHouse(basicShader, cube, roof, windowMesh, doorMesh, useTextureGlobally,
            woodTexture, roofTexture, stepTexture, windowGlassTexture, doorTexture);

        
        // -------------------- 绘制树木 --------------------
        for (auto& tree : trees) {
            float scale = tree.scale; // 获取随机缩放因子

            // 树干 - 应用随机高度
            glm::mat4 model = glm::translate(glm::mat4(1.0f), tree.position);
            model = glm::translate(model, glm::vec3(0.0f, 0.6f * scale, 0.0f)); // 高度随机
            model = glm::scale(model, glm::vec3(1.0f * scale, 1.2f * scale, 1.0f * scale)); // 整体随机缩放

            basicShader.setMat4("model", model);

            // 绑定树干纹理，设置纹理开关
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, barkTexture);
            basicShader.setBool("useTexture", useTextureGlobally);

            // 设置材质
            basicShader.setVec3("material.ambient", treeTrunkColor.ambient);
            basicShader.setVec3("material.specular", treeTrunkColor.specular);
            basicShader.setFloat("material.shininess", treeTrunkColor.shininess);

            // 漫反射颜色设置
            if (!useTextureGlobally) {
                basicShader.setVec3("material.diffuse", treeTrunkColor.diffuse);
            }
            else {
                basicShader.setVec3("material.diffuse", glm::vec3(1.0f));
            }

            glBindVertexArray(cylinder.VAO);
            glDrawElements(GL_TRIANGLES, cylinder.indexCount, GL_UNSIGNED_INT, 0);

            // 树冠 - 应用随机大小
            model = glm::translate(glm::mat4(1.0f), glm::vec3(tree.position.x, 1.2f * scale, tree.position.z));
            model = glm::scale(model, glm::vec3(0.8f * scale, 1.2f * scale, 0.8f * scale)); // 树冠随机缩放

            basicShader.setMat4("model", model);

            // 绑定树冠纹理，设置纹理开关
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, leavesTexture);
            basicShader.setBool("useTexture", useTextureGlobally);

            // 设置材质
            basicShader.setVec3("material.ambient", treeCrownColor.ambient);
            basicShader.setVec3("material.specular", treeCrownColor.specular);
            basicShader.setFloat("material.shininess", treeCrownColor.shininess);

            // 漫反射颜色设置
            if (!useTextureGlobally) {
                basicShader.setVec3("material.diffuse", treeCrownColor.diffuse);
            }
            else {
                basicShader.setVec3("material.diffuse", glm::vec3(1.0f));
            }

            glBindVertexArray(cone.VAO);
            glDrawElements(GL_TRIANGLES, cone.indexCount, GL_UNSIGNED_INT, 0);
        }

        // 绘制天空盒 (保持不变)
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.getView()));
        skyboxShader.setMat4("proj", proj);
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setInt("skybox", 0);

        glBindVertexArray(skybox.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, skybox.indexCount);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Scene Control");
        ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
        ImGui::Separator();

        // Global texture toggle
        ImGui::Text("Global Texture/Solid Color");
        ImGui::Checkbox("Enable Textures (Walls/Trees)", &useTextureGlobally);
        ImGui::Separator();

        ImGui::Text("Cabin Materials (Solid Color/Other)");
        ImGui::ColorEdit3("Wall Color", (float*)&woodColor.diffuse);
        ImGui::ColorEdit3("Roof Color", (float*)&roofColor.diffuse);
        ImGui::ColorEdit3("Window Color", (float*)&windowColor.diffuse);
        ImGui::ColorEdit3("Door Color", (float*)&doorColor.diffuse);
        ImGui::Separator();
        ImGui::Text("Tree Materials (Solid Color)");
        ImGui::ColorEdit3("Trunk Color", (float*)&treeTrunkColor.diffuse);
        ImGui::ColorEdit3("Crown Color", (float*)&treeCrownColor.diffuse);
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::SliderFloat3("Light Direction", (float*)&lightDir.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("Light Color", (float*)&lightColor);

        if (ImGui::Button(captureMouse ? "Release Mouse" : "Capture Mouse")) {
            captureMouse = !captureMouse;
            glfwSetInputMode(window, GLFW_CURSOR, captureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            camera.firstMouse = true;
        }
        ImGui::End();

        // 更新光照方向
        lightDir = glm::normalize(lightDir);

        // 渲染ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}