#include "Camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() 
    : pos(0.0f, 1.6f, 5.0f)
    , front(0.0f, 0.0f, -1.0f)
    , up(0.0f, 1.0f, 0.0f)
    , yaw(-90.0f)
    , pitch(0.0f)
    , speed(3.5f)
    , sensitivity(0.1f)
    , firstMouse(true)
    , lastX(400.0f)
    , lastY(300.0f)
{
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(pos, pos + front, up);
}

void Camera::processMouse(float xpos, float ypos) {
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

void Camera::processKeyboard(int direction, float deltaTime) {
    float velocity = speed * deltaTime;
    if (direction == 0) pos += front * velocity;
    if (direction == 1) pos -= front * velocity;
    if (direction == 2) pos -= glm::normalize(glm::cross(front, up)) * velocity;
    if (direction == 3) pos += glm::normalize(glm::cross(front, up)) * velocity;
}

