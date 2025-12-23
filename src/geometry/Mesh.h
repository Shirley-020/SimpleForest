#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

struct Mesh {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    GLsizei indexCount;

    Mesh();
};

#endif // MESH_H

