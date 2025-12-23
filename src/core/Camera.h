#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
    bool firstMouse;
    float lastX;
    float lastY;

    Camera();
    glm::mat4 getView() const;
    void processMouse(float xpos, float ypos);
    void processKeyboard(int direction, float deltaTime);
};

#endif // CAMERA_H

