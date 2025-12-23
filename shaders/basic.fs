#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord; // <-- ������������

// ���ʽṹ�� (���ֲ���)
struct Material {
    vec3 ambient;
    vec3 diffuse; 
    vec3 specular;
    float shininess;
}; 
uniform Material material;

uniform sampler2D texture_diffuse1; // <-- ��������������
uniform bool useTexture; // <-- �����Ƿ�ʹ�������Ŀ���

// ... (���սṹ�� Light ���ֲ���) ...

uniform vec3 lightDir; 
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    // 1. ��������ɫ���Ӳ��ʻ������л�ȡ
    vec3 diffuseColor;
    if (useTexture) {
        diffuseColor = vec3(texture(texture_diffuse1, TexCoord)); // ��������
    } else {
        diffuseColor = material.diffuse; // ʹ�ô�ɫ
    }

    // 2. ������（适当提高环境光，避免阴影过黑）
    vec3 ambient = material.ambient * lightColor * 1.2;

    // 3. ���������
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir); // �����Դ�����ǹ̶���
    float diffRaw = max(dot(norm, lightDirNorm), 0.0);
    // 防止刚稍微背光就完全变黑：给漫反射一个最小强度
    float diff = max(diffRaw, 0.2);
    vec3 diffuse = diff * lightColor * diffuseColor; // ʹ�� diffuseColor (������ɫ)

    // 4. �������
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = normalize(reflect(lightDirNorm, norm));  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * lightColor * material.specular;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}