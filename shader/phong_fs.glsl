#version 410 core
#define POINT_LIGHTS_NUM 2
#define DIR_LIGHTS_NUM 1

struct DirLight {
    vec3 direction;
    vec3 radiance;
};

struct PointLight {
    vec3 position;
    vec3 radiance;
    float placeholder;
    float constant;
    float linear;
    float quadratic;
};

layout (std140) uniform Lights {
    PointLight pointLights[POINT_LIGHTS_NUM];
    DirLight dirLights[DIR_LIGHTS_NUM];
};

uniform vec3 viewPos;
uniform sampler2D texSlot;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform int glossiness;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;
out vec4 FragColor;

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 color = texture(texSlot, fs_in.TexCoord).rgb;
    vec3 result = vec3(0.0);
    for (int i = 0; i < POINT_LIGHTS_NUM; i++) {
        PointLight light = pointLights[i];
        float distance = length(light.position - fs_in.FragPos);
        vec3 lightDir = normalize(light.position - fs_in.FragPos);
        vec3 ambient = ka * color;
        vec3 diffuse = kd * max(dot(lightDir, normal), 0.0) * color * light.radiance;
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), glossiness) * light.radiance;
        result += (ambient + diffuse + specular) / vec3(light.constant + light.linear * distance + light.quadratic * pow(distance, 2));
    }
    for (int i = 0; i < DIR_LIGHTS_NUM; i++) {
        DirLight light = dirLights[i];
        vec3 lightDir = normalize(-light.direction);
        vec3 ambient = ka * color;
        vec3 diffuse = kd * max(dot(lightDir, normal), 0.0) * color * light.radiance;
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), glossiness) * light.radiance;
        result += (ambient + diffuse + specular);
    }

    FragColor = vec4(result, 1.0) * pointLights[0].constant;
}
