#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    unsigned int ID;
    Shader();
    bool load(const char* vertPath, const char* fragPath);
    void use() const;
    void setMat4(const char* name, const glm::mat4& m) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setVec3(const char* name, const glm::vec3& v) const;
    void setFloat(const char* name, float f) const;
    void setInt(const char* name, int v) const;
    void setBool(const char* name, bool v) const;
private:
    bool checkCompile(unsigned int shader, const char* type);
};

#endif // SHADER_H

