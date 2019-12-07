#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 captureProjection;
uniform mat4 captureView;

void main()
{
    WorldPos = aPos;
    gl_Position = captureProjection * captureView * vec4(WorldPos, 1.0);
}