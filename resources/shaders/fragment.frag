#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(fragNormal), lightDir), 0.0);
    vec3 ambient = fragColor * 0.3;
    vec3 diffuse = fragColor * diff;
    outColor = vec4(ambient + diffuse, 1.0);
}