#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord; // 纹理坐标  

// 材质结构体
struct Material {
    vec3 ambient;
    vec3 diffuse; 
    vec3 specular;
    float shininess;
}; 
uniform Material material;// 材质属性
// 纹理采样器
uniform sampler2D texture_diffuse1; 
uniform bool useTexture; 


// 光源属性
uniform vec3 lightDir; 
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    
    vec3 diffuseColor;
    if (useTexture) { 
        diffuseColor = vec3(texture(texture_diffuse1, TexCoord)); // 从纹理中获取漫反射颜色
    } else {
        diffuseColor = material.diffuse; // 使用材质的漫反射颜色
    }

    //提高环境光强度
    vec3 ambient = material.ambient * lightColor * 1.0;

    
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir); 
    float diffRaw = max(dot(norm, lightDirNorm), 0.0);
   
    float diff = max(diffRaw, 0.2);
    vec3 diffuse = diff * lightColor * diffuseColor;  

    // 计算镜面反射分量
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = normalize(reflect(lightDirNorm, norm));  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * lightColor * material.specular;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}