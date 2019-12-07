#version 410 core
#define POINT_LIGHTS_NUM 2
#define DIR_LIGHTS_NUM 1
#define M_PI 3.14159265358979323846
#define INV_PI 0.31830988618379067154

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
uniform vec3 albedo;
uniform vec3 fresnel_0;
uniform float alpha;
uniform float metallic;

//uniform samplerCube environmentMap;
uniform samplerCube irradianceMap;
uniform samplerCube preFilterMap;
uniform sampler2D brdfLUT;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;
out vec4 FragColor;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float alpha) {
    float a2 = alpha * alpha;
    //float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
void main() {
    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), fresnel_0, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < POINT_LIGHTS_NUM; i++) {
        // calculate per-light radiance
        vec3 L = normalize(pointLights[i].position - fs_in.FragPos);
        vec3 H = normalize(V + L);
        vec3 radiance = pointLights[i].radiance;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, alpha);
        float G   = GeometrySmith(N, V, L, sqrt(alpha));
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        // add to outgoing radiance Lo
        Lo += (kd * INV_PI * albedo + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(preFilterMap, R,  sqrt(alpha) * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), sqrt(alpha))).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = specular + kd * diffuse;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
