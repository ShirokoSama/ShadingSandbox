#version 410 core
#define POINT_LIGHTS_NUM 2
#define DIR_LIGHTS_NUM 1
#define M_PI 3.14159265358979323846
#define INV_PI 0.31830988618379067154
#define INV_TWOPI 0.15915494309189533577
#define INV_FOURPI .07957747154594766788
#define SQRT_TWO 1.41421356237309504880
#define INV_SQRT_TWO 0.70710678118654752440
#define SAMPLE_COUNT 1024

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
uniform samplerCube environmentMap;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;
out vec4 FragColor;

struct CoordinateSystem {
    vec3 n;
    vec3 s;
    vec3 t;
};

CoordinateSystem getCoordSystem(vec3 n) {
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, n));
    vec3 tangentY = cross(n, tangentX);
    CoordinateSystem coord;
    coord.n = n;
    coord.s = tangentX;
    coord.t = tangentY;
    return coord;
}

vec3 toWorld(CoordinateSystem system, vec3 v) {
    return system.s * v.x + system.t * v.y + system.n * v.z;
}

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 squareToBeckmann(vec2 _sampler) {
	float ux = _sampler.x;
	float uy = _sampler.y;
	float phi = 2 * M_PI * ux;
	float theta = atan(sqrt(-alpha * alpha * log(1 - uy)));
	return vec3(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	);
}

vec3 squareToCosineHemisphere(vec2 _sampler) {
	float ux = _sampler.x;
	float uy = _sampler.y;
	float radius = sqrt(ux);
	float theta = 2.0 * M_PI * uy;
	vec2 diskSample = vec2(cos(theta) * radius, sin(theta) * radius);
	return vec3(
        diskSample.x,
        diskSample.y,
        sqrt(1.0 - diskSample.x * diskSample.x - diskSample.y * diskSample.y)
    );
}

float ndf(vec3 lm, vec3 n) {
    float cosTheta = dot(lm, n);
    float tanTheta2 = pow(cosTheta, -2) - 1;
    return exp(-tanTheta2 / pow(alpha, 2)) / (M_PI * pow(alpha, 2) * pow(cosTheta, 4));
}

float g1(vec3 v, vec3 m, vec3 n) {
    float chi = dot(v, m) / dot(v, n);
    float cosTheta = dot(v, n);
    float tanTheta = sqrt(1 - cosTheta * cosTheta) / cosTheta;
    float b = 1.0 / (alpha * tanTheta + 0.001);
    return min(step(0, chi) * (3.535 * b + 2.181 * b * b) / (1.0 + 2.276 * b + 2.577 * b * b), 1.0);
}

float gSmith(vec3 lo, vec3 li, vec3 lm, vec3 n) {
    return g1(li, lm, n) * g1(lo, lm, n);
}

vec3 fresnelShlick(vec3 f0, vec3 lo, vec3 lm) {
    return f0 + (vec3(1.0) - f0) * pow((1.0 - max(dot(lo, lm), 0.0)), 5);
}

vec3 evalMicrofacet(vec3 lo, vec3 li, vec3 n) {
    vec3 lm = normalize(lo + li);
    vec3 f0 = mix(vec3(0.04), fresnel_0, metallic);
    vec3 fresnel = fresnelShlick(f0, lo, lm);
    vec3 kd = (vec3(1.0) - fresnel) * (1.0 - metallic);
    vec3 diffuse = kd * INV_PI * albedo;
    vec3 specular = (ndf(lm, n) * gSmith(lo, li, lm, n) * fresnel) / (4.0 * dot(lo, n) * dot(li, n) + 0.001);
    return max((diffuse + max(specular, 0.0)) * vec3(dot(li, n)), 0.0);
}

//float pdf(vec3 wi, vec3 wo, vec3 n, float ks, float kd) {
//    if (dot(wi, n) <= 0 || dot(wo, n) <= 0)
//        return 0.0;
//    vec3 wh = normalize(wi + wo);
//    float d = ndf(wh, n);
//    float jh = 1.0 / (4 * dot(wh, wo));
//    return ks * d * jh + kd * dot(wo, n) * INV_PI;
//}

vec3 sampleMicrofacet(CoordinateSystem system, vec3 wi, inout vec3 wo, vec2 _sampler) {
    vec3 coefficient;
    vec3 microfacetNormal = toWorld(system, squareToBeckmann(_sampler));
    wo = normalize(reflect(-wi, microfacetNormal));
    vec3 f0 = mix(vec3(0.04), fresnel_0, metallic);
    vec3 nominator = fresnelShlick(f0, wi, microfacetNormal) * gSmith(wo, wi, microfacetNormal, system.n) * dot(microfacetNormal, wi);
    float denominator = dot(system.n, microfacetNormal) * dot(system.n, wi);
    coefficient = nominator / denominator;
    return coefficient;
}

vec3 sampleLambert(CoordinateSystem system, vec3 wi, inout vec3 wo, vec2 _sampler) {
    vec3 coefficient;
    vec3 lambertDiffuseDir = squareToCosineHemisphere(_sampler);
    wo = normalize(toWorld(system, lambertDiffuseDir));
    coefficient = albedo;
    return coefficient;
}

vec3 sampleLambert2(CoordinateSystem system, vec3 wi, inout vec3 wo, vec2 _sampler) {
    vec3 coefficient;
    float phi = 2.0 * M_PI * _sampler.x;
    float theta = 0.5 * M_PI * _sampler.y;
    vec3 sampleDir = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
    wo = normalize(toWorld(system, sampleDir));
    coefficient = albedo * (cos(theta) * sin(theta) * M_PI);
    return coefficient;
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 color = vec3(0.0);

    for (int i = 0; i < POINT_LIGHTS_NUM; i++) {
        PointLight light = pointLights[i];
        vec3 lightDir = normalize(light.position - fs_in.FragPos);
        color += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
    }

//    for (int i = 0; i < DIR_LIGHTS_NUM; i++) {
//        DirLight light = dirLights[i];
//        vec3 lightDir = normalize(-light.direction);
//        color += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
//    }

    if (dot(viewDir, normal) > 0) {
        CoordinateSystem sys = getCoordSystem(normal);
        vec3 sampleRslt = vec3(0.0);
        vec3 f0 = mix(vec3(0.04), fresnel_0, metallic);
        vec3 fresnel = fresnelShlick(f0, viewDir, normal);
        float ks = max(fresnel.x, max(fresnel.y, fresnel.z));
        float kd = (1.0 - ks) * (1.0 - metallic);
        ks = ks / (ks + kd);
        kd = 1.0 - ks;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            vec2 _sampler = Hammersley(i, SAMPLE_COUNT);
            vec3 wo;
            vec3 coefficient;
            if (_sampler.x < ks) {
                _sampler.x = _sampler.x / ks;
                coefficient = sampleMicrofacet(sys, viewDir, wo, _sampler);
            }
            else {
                _sampler.x = (_sampler.x - ks) / kd;
                coefficient = sampleLambert(sys, viewDir, wo, _sampler);
            }
            vec3 envColor = texture(environmentMap, wo).rgb;
            sampleRslt += coefficient * envColor;
        }
        sampleRslt /= SAMPLE_COUNT;
        color += sampleRslt;
    }

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
