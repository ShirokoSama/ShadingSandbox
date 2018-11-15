#version 410 core
#define POINT_LIGHTS_NUM 2
#define DIR_LIGHTS_NUM 1
#define M_PI 3.14159265358979323846
#define INV_PI 0.31830988618379067154
#define INV_TWOPI 0.15915494309189533577
#define INV_FOURPI .07957747154594766788
#define SQRT_TWO 1.41421356237309504880
#define INV_SQRT_TWO 0.70710678118654752440
#define SAMPLE_COUNT 32

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
	vec3 c;
	if (abs(n.x) > abs(n.y)) {
		float invLen = 1.0 / sqrt(n.x * n.x + n.z * n.z);
		c = vec3(n.z * invLen, 0.0, -n.x * invLen);
	}
	else {
		float invLen = 1.0f / sqrt(n.y * n.y + n.z * n.z);
		c = vec3(0.0, n.z * invLen, -n.y * invLen);
	}
	vec3 b = cross(c, n);
	CoordinateSystem coord;
	coord.n = n;
	coord.s = b;
	coord.t = c;
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
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

float ndf(vec3 lm, vec3 n) {
    float cosTheta = dot(lm, n);
    float tanTheta2 = pow(cosTheta, -2) - 1;
    return cosTheta > 0 ?
        (1.0 * INV_PI) *
        (exp(-tanTheta2 / pow(alpha, 2)) / pow(alpha, 2) * pow(cosTheta, 3))
        : 0.0;
}

float g1(vec3 v, vec3 m, vec3 n) {
    float chi = dot(v, m) / dot(v, n);
    if (chi > 0) {
        float cosTheta = dot(v, m);
        float tanTheta = sqrt(1 - cosTheta * cosTheta) / cosTheta;
        float b = 1.0 / (alpha * tanTheta);
        if (b < 1.6)
            return (3.535 * b + 2.181 * b * b) / (1.f + 2.276 * b + 2.577 * b * b);
        else
            return 1.0;
    }
    else
        return 0.0;
}

float gSmith(vec3 lo, vec3 li, vec3 lm, vec3 n) {
    return g1(li, lm, n) * g1(lo, lm, n);
}

vec3 fresnel(vec3 lo, vec3 lm) {
    return fresnel_0 + (vec3(1.0) - fresnel_0) * pow((1.0 - dot(lo, lm)), 5);
}

float pdf(vec3 wi, vec3 wo, vec3 n, float ks) {
	if (dot(wi, n) <= 0.0 || dot(wo, n) <= 0.0) {
		return 0.0;
	}
	vec3 wh = normalize(wi + wo);
	float d = ndf(wh, n);
	float jh = 1.0 / (4 * dot(wh, wo));
	return ks * d * jh + (1.0 - ks) * dot(wo, n) * INV_PI;
}

vec3 squareToBeckmann(vec2 _sample) {
	float ux = _sample.x;
	float uy = _sample.y;
	float phi = 2 * M_PI * ux;
	float theta = atan(sqrt(-alpha * alpha * log(uy)));
	return vec3(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	);
}

vec3 squareToCosineHemisphere(vec2 _sample) {
	float ux = _sample.x;
	float uy = _sample.y;
	float radius = sqrt(ux);
	float theta = 2.0 * M_PI * uy;
	vec2 diskSample = vec2(cos(theta) * radius, sin(theta) * radius);
	return vec3(diskSample.x, diskSample.y, sqrt(1.0 - diskSample.x * diskSample.x - diskSample.y * diskSample.y));
}

vec3 evalMicrofacet(vec3 lo, vec3 li, vec3 n, float ks) {
    vec3 lm = normalize(lo + li);
    vec3 diffuse = kd * INV_PI;
    vec3 specular = ks * (ndf(lm, n) * gSmith(lo, li, lm, n) * fresnel(lo, lm)) / (4.0 * dot(lo, n) * dot(li, n));
    return (diffuse + specular) * vec3(dot(lo, n));
}

vec3 sampleBrdf(CoordinateSystem system, vec3 wi, inout vec3 wo, vec2 _sample, float ks) {
	float ux = _sample.x;
	float uy = _sample.y;
	if (ux <= ks) {
		vec3 microfacetNormal = toWorld(system, squareToBeckmann(vec2(ux / ks, uy)));
		wo = normalize(reflect(-wi, microfacetNormal));
	}
	else {
		wo = normalize(toWorld(system, squareToCosineHemisphere(vec2((ux - ks) / (1.0 - ks), uy))));
	}
	return pdf(wi, wo, system.n, ks) == 0.0 ? vec3(0.0) :
		evalMicrofacet(wi, wo, system.n, ks) * dot(wo, system.n) / pdf(wi, wo, system.n, ks);
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);
	float ks = 1.0 - max(max(kd.x, kd.y), kd.z);

    for (int i = 0; i < POINT_LIGHTS_NUM; i++) {
        PointLight light = pointLights[i];
        vec3 lightDir = normalize(light.position - fs_in.FragPos);
        if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
            result += evalMicrofacet(viewDir, lightDir, normal, ks) * light.radiance;
    }
    for (int i = 0; i < DIR_LIGHTS_NUM; i++) {
        DirLight light = dirLights[i];
        vec3 lightDir = normalize(-light.direction);
        if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
            result += evalMicrofacet(viewDir, lightDir, normal, ks) * light.radiance;
    }

	if (dot(viewDir, normal) > 0) {
		CoordinateSystem sys = getCoordSystem(normal);
		vec3 sampleRslt = vec3(0.0);
		for (int i = 0; i < SAMPLE_COUNT; i++) {
			vec2 _sampler = Hammersley(i, SAMPLE_COUNT);
			vec3 wo;
			vec3 coefficient = sampleBrdf(sys, viewDir, wo, _sampler, ks);
			vec3 envColor = texture(environmentMap, wo).rgb;
			envColor = envColor / (envColor + vec3(1.0));
			envColor = pow(envColor, vec3(1.0 / 2.2));
			sampleRslt += coefficient * envColor * 2.0;
		}
		sampleRslt /= SAMPLE_COUNT;
		result += sampleRslt;
	}

    FragColor = vec4(result, 1.0);
}
