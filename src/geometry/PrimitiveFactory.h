#ifndef PRIMITIVE_FACTORY_H
#define PRIMITIVE_FACTORY_H

#include "Mesh.h"

Mesh createCube();
Mesh createCone(int segments = 20, float height = 1.0f, float radius = 0.8f);
Mesh createCylinder(int segments = 16, float height = 1.0f, float radius = 0.2f);
Mesh createWindow(float width = 0.3f, float height = 0.4f);
Mesh createDoor(float width = 0.5f, float height = 1.0f);
Mesh createRoof(float width = 4.0f, float depth = 5.0f, float pitch = 0.5f);
Mesh createSkybox();

#endif // PRIMITIVE_FACTORY_H

