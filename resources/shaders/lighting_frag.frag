#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float nearPlane;
    float farPlane;
    vec2 padding;
} perFrame;

layout(set = 1, binding = 0) uniform sampler2D gPosition;
layout(set = 1, binding = 1) uniform sampler2D gNormal;
layout(set = 1, binding = 2) uniform sampler2D gAlbedo;
layout(set = 1, binding = 3) uniform sampler2D gMaterial;
layout(set = 1, binding = 4) uniform sampler2D gDepth;

struct Light
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
    vec4 atten;
};

layout(set = 2, binding = 0) readonly buffer LightSSBO
{
    Light lights[1024];
    int lightCount;
} lightData;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outEmissive;

vec3 ComputeDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic)
{
    vec3 lightDir = normalize(light.direction.xyz);
    float intensity = light.direction.w;
    vec3 lightColor = light.color.rgb * intensity;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * metallic;

    return lightColor * diffuse + specular;
}

vec3 ComputePointLight(Light light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo, float metallic)
{
    vec3 lightPos = light.position.xyz;
    vec3 lightDir = lightPos - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    float intensity = light.direction.w;
    vec3 lightColor = light.color.rgb * intensity;

    float attenuation = 1.0 / (light.atten.x + light.atten.y * distance + light.atten.z * (distance * distance));
    float radius = light.params.z;
    float radiusAtten = clamp(1.0 - (distance * distance) / (radius * radius), 0.0, 1.0);
    radiusAtten *= radiusAtten;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * metallic;

    return (lightColor * diffuse + specular) * attenuation * radiusAtten;
}

vec3 ComputeSpotLight(Light light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo, float metallic)
{
    vec3 lightPos = light.position.xyz;
    vec3 lightDir = lightPos - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    float intensity = light.direction.w;
    vec3 lightColor = light.color.rgb * intensity;

    float attenuation = 1.0 / (light.atten.x + light.atten.y * distance + light.atten.z * (distance * distance));
    float radius = light.params.z;
    float radiusAtten = clamp(1.0 - (distance * distance) / (radius * radius), 0.0, 1.0);
    radiusAtten *= radiusAtten;

    vec3 spotDir = normalize(light.direction.xyz);
    float theta = dot(lightDir, spotDir);
    float innerCutoff = light.params.x;
    float outerCutoff = light.params.y;
    float epsilon = innerCutoff - outerCutoff;
    float spotIntensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * metallic;

    return (lightColor * diffuse + specular) * attenuation * radiusAtten * spotIntensity;
}

void main()
{
    vec4 posData = texture(gPosition, fragUV);
    vec3 worldPos = posData.xyz;
    if (posData.w < 0.001) discard;

    vec3 normal = normalize(texture(gNormal, fragUV).xyz);
    vec4 albedoData = texture(gAlbedo, fragUV);
    vec3 albedo = albedoData.rgb;
    float metallic = albedoData.a;

    vec4 matData = texture(gMaterial, fragUV);
    float roughness = matData.r;
    float ao = matData.g;
    float emissive = matData.b;

    vec3 viewDir = normalize(perFrame.cameraPos - worldPos);

    vec3 ambient = albedo * 0.3 * ao;
    vec3 lighting = vec3(0.0);

    for (int i = 0; i < lightData.lightCount && i < 1024; i++)
    {
        Light light = lightData.lights[i];
        int type = int(light.position.w);

        if (type == 0)
            lighting += ComputeDirectionalLight(light, normal, viewDir, albedo, metallic);
        else if (type == 1)
            lighting += ComputePointLight(light, worldPos, normal, viewDir, albedo, metallic);
        else if (type == 2)
            lighting += ComputeSpotLight(light, worldPos, normal, viewDir, albedo, metallic);
    }

    outColor = vec4(ambient + lighting, 1.0);
    outEmissive = vec4(albedo * emissive, 1.0);
}
