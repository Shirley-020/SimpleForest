#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include "../core/Camera.h"

extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern Camera camera;
extern bool keys[1024];
extern bool captureMouse;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(float dt);

#endif // INPUT_H

