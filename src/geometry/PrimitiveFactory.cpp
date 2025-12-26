#include "PrimitiveFactory.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

Mesh createCone(int segments, float height, float radius) {
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

Mesh createCylinder(int segments, float height, float radius) {
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

Mesh createWindow(float width, float height) {
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

Mesh createDoor(float width, float height) {
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

Mesh createRoof(float width, float depth, float pitch) {
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float roofHeight = width * pitch * 0.5f; // 基于宽度计算高度，pitch 是坡度

    // 四面体屋顶的4个顶点：
    // 顶点0: 底面左下 (-halfW, 0.0f, -halfD)
    // 顶点1: 底面右下 (halfW, 0.0f, -halfD)  
    // 顶点2: 底面右上 (halfW, 0.0f, halfD)
    // 顶点3: 底面左上 (-halfW, 0.0f, halfD)
    // 顶点4: 顶部中心 (0.0f, roofHeight, 0.0f)

    std::vector<float> vertices = {
        // ------------------ 四个三角形面 ------------------
        // 每个面3个顶点，共12个顶点
        // 格式：位置(3), 法线(3), 纹理坐标(2) -> 8个float

        // 前面 (顶点0,1,4)
        -halfW, 0.0f, -halfD,    0.0f, 0.447f, 0.894f,    0.0f, 0.0f,  // 左下
         halfW, 0.0f, -halfD,    0.0f, 0.447f, 0.894f,    1.0f, 0.0f,  // 右下
         0.0f, roofHeight, 0.0f, 0.0f, 0.447f, 0.894f,    0.5f, 1.0f,  // 顶部

         // 右面 (顶点1,2,4)
          halfW, 0.0f, -halfD,    0.894f, 0.447f, 0.0f,    0.0f, 0.0f,  // 右下
          halfW, 0.0f,  halfD,    0.894f, 0.447f, 0.0f,    1.0f, 0.0f,  // 右上
          0.0f, roofHeight, 0.0f, 0.894f, 0.447f, 0.0f,    0.5f, 1.0f,  // 顶部

          // 后面 (顶点2,3,4)
           halfW, 0.0f, halfD,     0.0f, 0.447f, -0.894f,   0.0f, 0.0f,  // 右上
          -halfW, 0.0f, halfD,     0.0f, 0.447f, -0.894f,   1.0f, 0.0f,  // 左上
           0.0f, roofHeight, 0.0f, 0.0f, 0.447f, -0.894f,   0.5f, 1.0f,  // 顶部

           // 左面 (顶点3,0,4)
           -halfW, 0.0f,  halfD,    -0.894f, 0.447f, 0.0f,   0.0f, 0.0f,  // 左上
           -halfW, 0.0f, -halfD,    -0.894f, 0.447f, 0.0f,   1.0f, 0.0f,  // 左下
            0.0f, roofHeight, 0.0f, -0.894f, 0.447f, 0.0f,   0.5f, 1.0f,  // 顶部

            // ------------------ 底面 (可选) ------------------
            // 四面体屋顶通常没有底面，但如果你想要可以加上
            // 底面是正方形，由两个三角形组成
            -halfW, 0.0f, -halfD,    0.0f, -1.0f, 0.0f,    0.0f, 0.0f,  // 左下
             halfW, 0.0f, -halfD,    0.0f, -1.0f, 0.0f,    1.0f, 0.0f,  // 右下
             halfW, 0.0f,  halfD,    0.0f, -1.0f, 0.0f,    1.0f, 1.0f,  // 右上
            -halfW, 0.0f,  halfD,    0.0f, -1.0f, 0.0f,    0.0f, 1.0f,  // 左上
    };

    // 索引数据
    std::vector<unsigned int> indices = {
        // 四个侧面三角形
        0, 1, 2,    // 前面
        3, 4, 5,    // 右面
        6, 7, 8,    // 后面
        9, 10, 11,  // 左面

        // 底面 (两个三角形，可选)
        12, 13, 14,  // 底面三角形1
        14, 15, 12   // 底面三角形2
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
    float scale = 1000.0f;
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

