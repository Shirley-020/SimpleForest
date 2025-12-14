#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

// 变量名必须和代码里的 setMat4("proj", ...) 一致 → 用 proj 而非 projection
uniform mat4 proj;
// 变量名必须和代码里的 setMat4("view", ...) 一致 → 用 view 而非 viewMatrix
uniform mat4 view;

void main() {
    TexCoords = aPos;
    gl_Position = proj * view * vec4(aPos, 1.0);
}