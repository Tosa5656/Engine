#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragAlbedo;
layout(location = 3) out float fragMetallic;
layout(location = 4) out float fragRoughness;
layout(location = 5) out float fragAO;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO
{
    mat4 model;
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float normalStrength;
} perObject;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(perObject.model))) * perObject.normalStrength;
    fragNormal = normalMatrix * inNormal;
    fragUV = inUV;
    fragAlbedo = perObject.albedo;
    fragMetallic = perObject.metallic;
    fragRoughness = perObject.roughness;
    fragAO = perObject.ao;

    mat4 mvp = perFrame.proj * perFrame.view * perObject.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
}