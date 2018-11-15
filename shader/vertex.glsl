#version 410 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 projection;
};
uniform mat4 model;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_out;

void main() {
    vs_out.FragPos = vec3(model * vec4(vPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * vNormal;
    vs_out.TexCoord = vTexCoord;
    gl_Position = projection * view * model * vec4(vPos, 1.0);
}