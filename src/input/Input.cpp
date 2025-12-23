#include "Input.h"
#include <glad/glad.h>

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
Camera camera;
bool keys[1024] = { false };
bool captureMouse = true;

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

