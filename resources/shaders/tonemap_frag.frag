#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D hdrColor;

layout(set = 1, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float nearPlane;
    float farPlane;
    float exposure;
    float pad;
} perFrame;

layout(location = 0) out vec4 outColor;

vec3 ACESFilmicToneMap(vec3 color)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 color = texture(hdrColor, fragUV).rgb * perFrame.exposure;
    color = ACESFilmicToneMap(color);
    outColor = vec4(color, 1.0);
}
