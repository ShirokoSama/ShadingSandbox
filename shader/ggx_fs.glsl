#version 410 core
#define POINT_LIGHTS_NUM 2
#define DIR_LIGHTS_NUM 1
#define M_PI 3.14159265358979323846
#define INV_PI 0.31830988618379067154
#define INV_TWOPI 0.15915494309189533577
#define INV_FOURPI .07957747154594766788
#define SQRT_TWO 1.41421356237309504880
#define INV_SQRT_TWO 0.70710678118654752440

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
uniform vec3 kd;
uniform vec3 fresnel_0;
uniform float alpha;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;
out vec4 FragColor;

float ndf(vec3 lm, vec3 n) {
    float cosTheta = dot(lm, n);
    float tanTheta2 = pow(cosTheta, -2) - 1;
    float alpha2 = pow(alpha, 2);
    return (alpha2 * INV_PI) / (pow(cosTheta, 4) * pow((alpha2 + tanTheta2), 2));
}

float g1(vec3 v, vec3 n) {
    float alpha2 = pow(alpha, 2);
    return (2 * dot(v, n)) / (dot(v, n) + sqrt(alpha2 + (1 - alpha2) * pow(dot(v, n), 2)));
}

float gSmith(vec3 lo, vec3 li, vec3 lm, vec3 n) {
    return g1(li, n) * g1(lo, n);
}

vec3 fresnel(vec3 lo, vec3 lm) {
    return fresnel_0 + (vec3(1.0) - fresnel_0) * pow((1.0 - dot(lo, lm)), 5);
}

vec3 evalMicrofacet(vec3 lo, vec3 li, vec3 n) {
    vec3 lm = normalize(lo + li);
    float ks = 1.0 - max(max(kd.x, kd.y), kd.z);
    vec3 diffuse = kd * INV_PI * texture(texSlot, fs_in.TexCoord).rgb;
    vec3 specular = ks * (ndf(lm, n) * gSmith(lo, li, lm, n) * fresnel(lo, lm)) / (4.0 * dot(lo, n) * dot(li, n));
    return (diffuse + specular) * vec3(dot(lo, n));
}

void main() {

    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);

    for (int i = 0; i < POINT_LIGHTS_NUM; i++) {
        PointLight light = pointLights[i];
        vec3 lightDir = normalize(light.position - fs_in.FragPos);
        if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
            result += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
    }
    for (int i = 0; i < DIR_LIGHTS_NUM; i++) {
        DirLight light = dirLights[i];
        vec3 lightDir = normalize(-light.direction);
        if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
            result += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
    }

    FragColor = vec4(result, 1.0);
}
