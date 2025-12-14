#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord; // <-- 接收纹理坐标

// 材质结构体 (保持不变)
struct Material {
    vec3 ambient;
    vec3 diffuse; 
    vec3 specular;
    float shininess;
}; 
uniform Material material;

uniform sampler2D texture_diffuse1; // <-- 新增纹理采样器
uniform bool useTexture; // <-- 控制是否使用纹理的开关

// ... (光照结构体 Light 保持不变) ...

uniform vec3 lightDir; 
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    // 1. 漫反射颜色：从材质或纹理中获取
    vec3 diffuseColor;
    if (useTexture) {
        diffuseColor = vec3(texture(texture_diffuse1, TexCoord)); // 采样纹理
    } else {
        diffuseColor = material.diffuse; // 使用纯色
    }

    // 2. 环境光
    vec3 ambient = material.ambient * lightColor;

    // 3. 漫反射光照
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir); // 假设光源方向是固定的
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * diffuseColor; // 使用 diffuseColor (纹理或纯色)

    // 4. 镜面光照
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = normalize(reflect(lightDirNorm, norm));  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * lightColor * material.specular;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}