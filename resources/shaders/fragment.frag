#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragAlbedo;
layout(location = 3) in float fragMetallic;
layout(location = 4) in float fragRoughness;
layout(location = 5) in float fragAO;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 albedo = texture(texSampler, fragUV).rgb;

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 viewDir = vec3(0.0, 0.0, 1.0);
    vec3 normal = normalize(fragNormal);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0 * (1.0 - fragRoughness));

    vec3 ambient = albedo * 0.3 * fragAO;
    vec3 diffuse = albedo * diff;

    float fresnel = fragMetallic * pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    vec3 specular = vec3(1.0) * spec * (1.0 - fragRoughness) * (1.0 - fragMetallic);
    vec3 metallicReflect = albedo * fragMetallic * fresnel;

    outColor = vec4(ambient + diffuse + specular + metallicReflect, 1.0);
}