#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragColor;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO
{
    mat4 model;
    vec3 color;
    float roughness;
    vec3 padding;
} perObject;

void main()
{
    mat4 mvp = perFrame.proj * perFrame.view * perObject.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    fragNormal = mat3(transpose(inverse(perObject.model))) * inNormal;
    fragUV = inUV;
    fragColor = perObject.color;
}