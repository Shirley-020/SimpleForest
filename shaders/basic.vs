#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord; // 纹理坐标

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord; // 传递给片段着色器

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoord = aTexCoord; //  传递纹理坐标

    gl_Position = proj * view * vec4(FragPos, 1.0);
}