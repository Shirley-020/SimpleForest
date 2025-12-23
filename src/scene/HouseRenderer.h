#ifndef HOUSE_RENDERER_H
#define HOUSE_RENDERER_H

#include "../core/Shader.h"
#include "../geometry/Mesh.h"

void renderDetailedHouse(Shader& shader, Mesh& cube, Mesh& roof, Mesh& windowMesh, Mesh& doorMesh,
    bool useTexture, unsigned int woodTex, unsigned int roofTex, unsigned int stepTex, 
    unsigned int windowTex, unsigned int doorTex);

#endif // HOUSE_RENDERER_H

