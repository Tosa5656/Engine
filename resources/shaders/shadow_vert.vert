#version 460

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float nearPlane;
    float farPlane;
    float exposure;
    float pad;
    mat4 lightSpaceMatrix;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO
{
    mat4 model;
    vec4 material;
} perObject;

void main()
{
    gl_Position = perFrame.lightSpaceMatrix * perObject.model * vec4(inPosition, 1.0);
}
