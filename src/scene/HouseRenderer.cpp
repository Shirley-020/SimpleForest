#include "HouseRenderer.h"
#include "Materials.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

