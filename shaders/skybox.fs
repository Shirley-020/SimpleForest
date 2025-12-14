#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

// 变量名必须和代码里的 setInt("skybox", 0) 一致 → 用 skybox 而非 cubeMap
uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, TexCoords);
}