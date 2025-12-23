#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <vector>
#include <string>

unsigned int loadTexture(const char* path);
unsigned int loadCubemap(const std::vector<std::string>& faces);

#endif // TEXTURE_H

