#ifndef MATERIALS_H
#define MATERIALS_H

#include <glm/glm.hpp>

struct Color {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

// 材质定义
extern Color woodColor;
extern Color roofColor;
extern Color windowColor;
extern Color doorColor;
extern Color treeTrunkColor;
extern Color treeCrownColor;
extern Color chimneyColor;
extern Color stepColor;

#endif // MATERIALS_H

