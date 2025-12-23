#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Core modules
#include "core/Shader.h"
#include "core/Camera.h"
#include "core/Texture.h"
#include "core/Model.h"
#include "core/PathUtils.h"

// Geometry modules
#include "geometry/Mesh.h"
#include "geometry/PrimitiveFactory.h"

// Scene modules
#include "scene/Materials.h"
#include "scene/Tree.h"
#include "scene/HouseRenderer.h"

// Input module
#include "input/Input.h"

// 全局变量
float deltaTime = 0.0f, lastFrame = 0.0f;
bool useTextureGlobally = true;

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
    if (!basicShader.load("shaders/basic.vs", "shaders/basic.fs")) {
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

    // 加载纹理（使用路径工具函数）
    std::vector<std::string> skyboxFaces = {
        getResourcePath("objects/right.jpg"),
        getResourcePath("objects/left.jpg"),
        getResourcePath("objects/top.jpg"),
        getResourcePath("objects/bottom.jpg"),
        getResourcePath("objects/front.jpg"),
        getResourcePath("objects/back.jpg")
    };
    unsigned int skyboxTexture = loadCubemap(skyboxFaces);

    // 加载新的纹理文件
	unsigned int woodTexture = loadTexture(getResourcePath("objects/wood_texture.jpg").c_str());   // 木纹纹理
	unsigned int barkTexture = loadTexture(getResourcePath("objects/bark_texture.jpg").c_str());    //  树干纹理
	unsigned int leavesTexture = loadTexture(getResourcePath("objects/leaves_texture.jpg").c_str());   // 树叶纹理
    unsigned int roofTexture = loadTexture(getResourcePath("objects/roof_shingles.jpg").c_str());        // 屋顶瓦片纹理
    unsigned int stepTexture = loadTexture(getResourcePath("objects/stone_step.jpg").c_str());          // 石头台阶纹理
    unsigned int windowGlassTexture = loadTexture(getResourcePath("objects/window_glass.jpg").c_str()); // 玻璃纹理 
	unsigned int doorTexture = loadTexture(getResourcePath("objects/door.jpg").c_str());              // 木门纹理
    
    // 光照参数
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f));
    glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);

    // 投影矩阵
    glm::mat4 proj = glm::perspective(glm::radians(75.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 500.0f);

    // 生成随机树木位置
    // 参数说明：树数量, x范围, z范围, 房子X范围(缓冲区), 房子Z范围(缓冲区), 树之间最小距离
    std::vector<Tree> trees = generateRandomTrees(
        40, -200.0f, 200.0f, -200.0f, 200.0f, 60.0f, 60.0f, 50.0f);

    // 加载树模型
    Model* treeModel = nullptr;
#ifdef ASSIMP_AVAILABLE
    std::cout << "=== Assimp is AVAILABLE, attempting to load tree model ===" << std::endl;
    std::string treeModelPath = getResourcePath("objects/tree.obj");
    std::cout << "Looking for tree model at: " << treeModelPath << std::endl;
    
    // 检查文件是否存在
    std::ifstream fileCheck(treeModelPath);
    if (!fileCheck.good()) {
        std::cerr << "ERROR: Tree model file not found: " << treeModelPath << std::endl;
        std::cerr << "Please ensure tree.obj exists in the objects directory." << std::endl;
    } else {
        fileCheck.close();
        std::cout << "Tree model file found. Loading..." << std::endl;
    }
    
    try {
        treeModel = new Model(treeModelPath);
        if (treeModel->meshes.empty()) {
            std::cerr << "Warning: Tree model loaded but has no meshes. Using procedural trees." << std::endl;
            delete treeModel;
            treeModel = nullptr;
        } else {
            std::cout << "=== Tree model loaded successfully! ===" << std::endl;
            std::cout << "  Path: " << treeModelPath << std::endl;
            std::cout << "  Meshes: " << treeModel->meshes.size() << std::endl;
            std::cout << "  Textures: " << treeModel->textures_loaded.size() << std::endl;
            std::cout << "  Scale factor: " << treeModel->scaleFactor << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception while loading tree model: " << e.what() << std::endl;
        std::cerr << "Using procedural trees." << std::endl;
        treeModel = nullptr;
    } catch (...) {
        std::cerr << "ERROR: Unknown exception while loading tree model." << std::endl;
        std::cerr << "Using procedural trees." << std::endl;
        treeModel = nullptr;
    }
#else
    std::cout << "=== Assimp is NOT available ===" << std::endl;
    std::cout << "Model loading is disabled. Using procedural trees." << std::endl;
    std::cout << "To enable model loading, please install Assimp library and reconfigure CMake." << std::endl;
#endif

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
        basicShader.setVec3("lightDir", lightDir);
        basicShader.setVec3("lightColor", lightColor);
        basicShader.setVec3("viewPos", camera.pos);
        basicShader.setMat4("proj", proj);
        basicShader.setMat4("view", camera.getView());

        // -------------------- 绘制小屋 --------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        basicShader.setInt("texture_diffuse1", 0);

        renderDetailedHouse(basicShader, cube, roof, windowMesh, doorMesh, useTextureGlobally,
            woodTexture, roofTexture, stepTexture, windowGlassTexture, doorTexture);

        
        // -------------------- 绘制树木 --------------------
        if (treeModel != nullptr) {
            // 使用加载的模型渲染树木
            for (auto& tree : trees) {
                float scale = tree.scale; // 随机缩放因子
                float finalScale = scale * 20.0f; // 现有比例因子（可调整）

                // 读取模型边界（模型已在加载时计算 bounding box）
                glm::vec3 modelMin = treeModel->getBoundingBoxMin(); // 本地模型坐标系下最小点
                float modelScaleFactor = treeModel->scaleFactor;     // 加载时 normalize 得到的 scaleFactor

                // 计算使模型底部贴地的 y 偏移（考虑 normalize 与最终缩放）
                float yOffset = -modelMin.y * modelScaleFactor * finalScale;

                // 将模型先移动到目标位置（包含 yOffset），再缩放
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(tree.position.x, tree.position.y + 1.5 * yOffset, tree.position.z));
                
                // 应用缩放
                model = glm::scale(model, glm::vec3(finalScale));
                
                basicShader.setMat4("model", model);
                
                // 设置材质（使用树干材质作为默认）
                basicShader.setVec3("material.ambient", treeTrunkColor.ambient);
                basicShader.setVec3("material.specular", treeTrunkColor.specular);
                basicShader.setFloat("material.shininess", treeTrunkColor.shininess);
                
                // 绑定纹理（优先使用模型自带的纹理，如果模型没有纹理则使用树叶纹理作为后备）
                glActiveTexture(GL_TEXTURE0);
                if (!treeModel->textures_loaded.empty()) {
                    glBindTexture(GL_TEXTURE_2D, treeModel->textures_loaded[0].id); // 使用模型自带的纹理
                } else {
                    glBindTexture(GL_TEXTURE_2D, leavesTexture); // 后备纹理
                }
                basicShader.setInt("texture_diffuse1", 0);
                basicShader.setBool("useTexture", useTextureGlobally);
                
                // 漫反射颜色设置
                if (!useTextureGlobally) {
                    basicShader.setVec3("material.diffuse", treeTrunkColor.diffuse);
                } else {
                    basicShader.setVec3("material.diffuse", glm::vec3(1.0f));
                }
                
                // 绘制模型
                treeModel->Draw(basicShader);
            }
        } else {
            // 使用原有的程序化几何体渲染树木（保持向后兼容）
            for (auto& tree : trees) {
                float scale = tree.scale; // 获取随机缩放因子

                // 树干 - 应用随机高度
                glm::mat4 model = glm::translate(glm::mat4(1.0f), tree.position);
                model = glm::translate(model, glm::vec3(0.0f, 0.2f * scale, 0.0f)); // 高度随机
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
        }

        // 绘制天空盒
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
    if (treeModel != nullptr) {
        delete treeModel;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
