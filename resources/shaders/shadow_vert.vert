#version 460

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConsts {
    int cascadeIndex;
} push;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float nearPlane;
    float farPlane;
    float exposure;
    int pcfKernel;
    mat4 lightSpaceMatrix;
    float normalOffsetBias;
    float depthBias;
    vec2 _pad0;
    mat4 lightSpaceMatrices[2];
    vec4 cascadeSplits;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO
{
    mat4 model;
    vec4 material;
} perObject;

void main()
{
    mat4 lightMatrix = (push.cascadeIndex == 0)
        ? perFrame.lightSpaceMatrix
        : perFrame.lightSpaceMatrices[push.cascadeIndex - 1];
    gl_Position = lightMatrix * perObject.model * vec4(inPosition, 1.0);
}
