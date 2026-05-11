#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D lightingResult;
layout(set = 1, binding = 0) uniform sampler2D emissiveAccum;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lighting = texture(lightingResult, fragUV).rgb;
    vec3 emissive = texture(emissiveAccum, fragUV).rgb;

    vec3 color = lighting + emissive;

    outColor = vec4(color, 1.0);
}
