#include "Model.h"
#include "Texture.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <climits>
#include <fstream>
#include <cstddef>

#ifdef ASSIMP_AVAILABLE
Model::Model(const std::string& path) : scaleFactor(1.0f) {
    loadModel(path);
}

void Model::loadModel(const std::string& path) {
    std::cout << "[Model] Loading model from: " << path << std::endl;
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        std::cerr << "Failed to load model: " << path << std::endl;
        if (!scene) {
            std::cerr << "  Scene is null!" << std::endl;
        }
        if (scene && (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
            std::cerr << "  Scene is incomplete!" << std::endl;
        }
        if (scene && !scene->mRootNode) {
            std::cerr << "  Root node is null!" << std::endl;
        }
        meshes.clear();
        textures_loaded.clear();
        return;
    }
    
    std::cout << "[Model] Scene loaded successfully. Meshes: " << scene->mNumMeshes << std::endl;

    directory = path.substr(0, path.find_last_of('/'));
    if (directory.empty()) {
        directory = path.substr(0, path.find_last_of('\\'));
    }
    if (directory.empty()) {
        directory = ".";
    }

    // 计算边界框并归一化
    calculateBoundingBox(scene);
    normalizeModel();

    processNode(scene->mRootNode, scene);
    
    std::cout << "[Model] Model processing complete. Total meshes: " << meshes.size() << std::endl;
}
#else
Model::Model(const std::string& path) : scaleFactor(1.0f) {
    std::cerr << "ERROR: Assimp not available. Cannot load model: " << path << std::endl;
    meshes.clear();
    textures_loaded.clear();
}

void Model::loadModel(const std::string& path) {
    std::cerr << "ERROR: Assimp not available. Cannot load model: " << path << std::endl;
    meshes.clear();
    textures_loaded.clear();
}
#endif

#ifdef ASSIMP_AVAILABLE
void Model::calculateBoundingBox(const aiScene* scene) {
    boundingBoxMin = glm::vec3(FLT_MAX);
    boundingBoxMax = glm::vec3(-FLT_MAX);

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            aiVector3D pos = mesh->mVertices[j];
            glm::vec3 vertex(pos.x, pos.y, pos.z);
            
            boundingBoxMin.x = std::min(boundingBoxMin.x, vertex.x);
            boundingBoxMin.y = std::min(boundingBoxMin.y, vertex.y);
            boundingBoxMin.z = std::min(boundingBoxMin.z, vertex.z);
            
            boundingBoxMax.x = std::max(boundingBoxMax.x, vertex.x);
            boundingBoxMax.y = std::max(boundingBoxMax.y, vertex.y);
            boundingBoxMax.z = std::max(boundingBoxMax.z, vertex.z);
        }
    }
}
#endif

void Model::normalizeModel() {
#ifdef ASSIMP_AVAILABLE
    glm::vec3 size = boundingBoxMax - boundingBoxMin;
    float maxSize = std::max({size.x, size.y, size.z});
    
    if (maxSize > 0.0f) {
        // 归一化到 [-1, 1] 范围，然后可以按需缩放
        scaleFactor = 2.0f / maxSize;
    } else {
        scaleFactor = 1.0f;
    }
#else
    scaleFactor = 1.0f;
#endif
}

#ifdef ASSIMP_AVAILABLE
void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh processedMesh = processMesh(mesh, scene);
        if (processedMesh.indexCount > 0) {
            meshes.push_back(processedMesh);
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // 处理顶点
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // 位置（应用缩放）
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x * scaleFactor;
        vector.y = mesh->mVertices[i].y * scaleFactor;
        vector.z = mesh->mVertices[i].z * scaleFactor;
        vertex.Position = vector;

        // 法线
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        } else {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // 纹理坐标
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // 处理索引
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, 
            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    // 创建 Mesh 对象
    Mesh result;
    
    glGenVertexArrays(1, &result.VAO);
    glGenBuffers(1, &result.VBO);
    glGenBuffers(1, &result.EBO);

    glBindVertexArray(result.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, result.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
        &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
        &indices[0], GL_STATIC_DRAW);

    // 顶点位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // 法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
        (void*)offsetof(Vertex, Normal));
    
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
        (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);

    result.indexCount = indices.size();
    
    // 存储纹理信息（简化处理，只使用第一个纹理）
    if (!textures.empty()) {
        // 纹理 ID 可以通过 shader 设置，这里我们只存储引用
    }

    return result;
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, 
    aiTextureType type, const std::string& typeName) {
    std::vector<Texture> textures;
    
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        
        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    
    return textures;
}
#endif

unsigned int Model::TextureFromFile(const char* path, const std::string& directory) {
    std::string filename = std::string(path);
    
    // 处理相对路径和绝对路径
    std::string fullPath;
    
    // 检查是否为绝对路径（Windows: 以盘符开头，Unix: 以/开头）
    bool isAbsolute = false;
#ifdef _WIN32
    if (filename.length() >= 2 && filename[1] == ':') {
        isAbsolute = true;
    }
#else
    if (filename.length() > 0 && filename[0] == '/') {
        isAbsolute = true;
    }
#endif
    
    if (isAbsolute) {
        fullPath = filename;
    } else {
        fullPath = directory + "/" + filename;
        // Windows 路径分隔符处理
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    }
    
    return loadTexture(fullPath.c_str());
}

void Model::Draw(const Shader& shader) const {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        glBindVertexArray(meshes[i].VAO);
        glDrawElements(GL_TRIANGLES, meshes[i].indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Model::DrawInstanced(const Shader& shader, const std::vector<glm::mat4>& modelMatrices) const {
    // 简化版本：循环绘制每个实例
    for (const auto& modelMatrix : modelMatrices) {
        shader.setMat4("model", modelMatrix);
        Draw(shader);
    }
}
