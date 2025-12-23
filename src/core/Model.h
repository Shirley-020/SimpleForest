#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "../geometry/Mesh.h"
#include "Shader.h"

#ifdef ASSIMP_AVAILABLE
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#else
// 前向声明，允许在没有 Assimp 时编译
namespace Assimp {
    class Importer;
}
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
#endif

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Model {
public:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;
    float scaleFactor; // 模型缩放因子

    Model(const std::string& path);
    void Draw(const Shader& shader) const;
    void DrawInstanced(const Shader& shader, const std::vector<glm::mat4>& modelMatrices) const;
    glm::vec3 getBoundingBoxMin() const { return boundingBoxMin; }
    glm::vec3 getBoundingBoxMax() const { return boundingBoxMax; }

private:
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    
    void loadModel(const std::string& path);
#ifdef ASSIMP_AVAILABLE
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    void calculateBoundingBox(const aiScene* scene);
#endif
    unsigned int TextureFromFile(const char* path, const std::string& directory);
    void normalizeModel();
};

#endif // MODEL_H

