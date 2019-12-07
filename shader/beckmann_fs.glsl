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

float ndf(vec3 lm, vec3 n) {
	float cosTheta = dot(lm, n);
	float tanTheta2 = pow(cosTheta, -2) - 1;
	return step(0, cosTheta) * exp(-tanTheta2 / pow(alpha, 2)) / (M_PI * pow(alpha, 2) * pow(cosTheta, 4));
}

float g1(vec3 v, vec3 m, vec3 n) {
	float chi = dot(v, m) / dot(v, n);
	float cosTheta = dot(v, n);
	float tanTheta = sqrt(1 - cosTheta * cosTheta) / cosTheta;
	float b = 1.0 / (alpha * tanTheta);
	return min(step(0, chi) * (3.535 * b + 2.181 * b * b) / (1.0 + 2.276 * b + 2.577 * b * b), 1.0);
}

float gSmith(vec3 lo, vec3 li, vec3 lm, vec3 n) {
	return g1(li, lm, n) * g1(lo, lm, n);
}

vec3 fresnelShlick(vec3 f0, vec3 lo, vec3 lm) {
	return f0 + (vec3(1.0) - f0) * pow((1.0 - dot(lo, lm)), 5);
}

vec3 evalMicrofacet(vec3 lo, vec3 li, vec3 n) {
    vec3 lm = normalize(lo + li);
	vec3 f0 = mix(vec3(0.04), fresnel_0, metallic);
	vec3 fresnel = fresnelShlick(f0, lo, lm);
	vec3 kd = (vec3(1.0) - fresnel) * (1.0 - metallic);
    vec3 diffuse = kd * INV_PI * albedo;
    vec3 specular = (ndf(lm, n) * gSmith(lo, li, lm, n) * fresnel) / (4.0 * dot(lo, n) * dot(li, n));
    return (diffuse + max(specular, 0.0)) * vec3(dot(li, n));
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 color = vec3(0.0);

	for (int i = 0; i < POINT_LIGHTS_NUM; i++) {
		PointLight light = pointLights[i];
		vec3 lightDir = normalize(light.position - fs_in.FragPos);
		// step函数如果作为累乘项会在n卡失效，集成显卡有效，这里为了用n卡直接写if语句了，用max做截断也可以消除分支
		if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
			color += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
	}

	for (int i = 0; i < DIR_LIGHTS_NUM; i++) {
		DirLight light = dirLights[i];
		vec3 lightDir = normalize(-light.direction);
		if (dot(lightDir, normal) > 0 && dot(viewDir, normal) > 0)
			color += evalMicrofacet(viewDir, lightDir, normal) * light.radiance;
	}

    FragColor = vec4(color, 1.0);
}
