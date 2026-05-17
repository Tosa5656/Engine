#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec2 fragUVAtlas;
layout(location = 3) in vec3 fragAlbedo;
layout(location = 4) in float fragMetallic;
layout(location = 5) in float fragRoughness;
layout(location = 6) in float fragAO;
layout(location = 7) in vec3 fragTangent;
layout(location = 8) in vec3 fragBitangent;
layout(location = 9) in vec3 fragWorldPos;

layout(set = 0, binding = 0) uniform PerFrameUBO {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float nearPlane;
    float farPlane;
    vec2 padding;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO {
    mat4 model;
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float normalStrength;
    vec2 uvOffset;
    vec2 uvScale;
    int parallaxMode;
    float parallaxScale;
    int parallaxIterations;
    float alphaCutoff;
    int alphaMode;
} perObject;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 0) uniform sampler2D texNormalSampler;
layout(set = 4, binding = 0) uniform sampler2D texHeightSampler;

struct Light {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
    vec4 atten;
    mat4 shadowMatrix;
};

layout(set = 5, binding = 0) readonly buffer LightSSBO {
    Light lights[1024];
    int lightCount;
} lightData;

layout(set = 5, binding = 1) readonly buffer ClusterCountSSBO {
    uint counts[];
} clusterCount;

layout(set = 5, binding = 2) readonly buffer ClusterIndexSSBO {
    uint indices[];
} clusterIndex;

layout(set = 5, binding = 3) uniform ClusterGridInfoUBO {
    uint tileCountX;
    uint tileCountY;
    uint clusterCount;
    uint depthSlices;
} gridInfo;

layout(set = 5, binding = 4) uniform sampler2D depthSampler;

layout(location = 0) out vec4 outColor;

vec2 ParallaxOcclusionMapping(vec2 texCoord, vec3 viewDirTS, float scale)
{
    int layers = max(perObject.parallaxIterations, 1);
    float layerDepth = 1.0 / float(layers);
    float currentLayerDepth = 0.0;

    vec2 P = viewDirTS.xy * scale;
    vec2 deltaTexCoords = P / float(layers);

    vec2 currentTexCoords = texCoord;
    float currentDepthMapValue = texture(texHeightSampler, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(texHeightSampler, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texHeightSampler, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);

    return mix(currentTexCoords, prevTexCoords, weight);
}

vec2 ReliefMapping(vec2 texCoord, vec3 viewDirTS, float scale)
{
    int iterations = max(perObject.parallaxIterations, 1);

    vec3 ray = normalize(vec3(viewDirTS.xy * scale, -viewDirTS.z));
    float depth = 0.0;
    float step = scale / float(iterations);

    for (int i = 0; i < iterations; i++)
    {
        vec2 sampleCoord = texCoord - ray.xy * depth;
        float h = texture(texHeightSampler, sampleCoord).r;
        if (h > depth) depth += step;
    }

    float binaryStep = step * 0.5;
    for (int i = 0; i < 5; i++)
    {
        vec2 sampleCoord = texCoord - ray.xy * depth;
        float h = texture(texHeightSampler, sampleCoord).r;
        if (h > depth) depth += binaryStep;
        else depth -= binaryStep;
        binaryStep *= 0.5;
    }

    return texCoord - ray.xy * depth;
}

vec3 ComputeDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.direction.xyz);
    float intensity = light.direction.w;
    vec3 lightColor = light.color.rgb * intensity;

    vec3 toLight = -lightDir;
    float diff = max(dot(normal, toLight), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(toLight + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * fragMetallic;

    return lightColor * diffuse + specular;
}

vec3 ComputePointLight(Light light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo)
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
    vec3 specular = lightColor * spec * fragMetallic;

    return (lightColor * diffuse + specular) * attenuation * radiusAtten;
}

vec3 ComputeSpotLight(Light light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo)
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
    vec3 specular = lightColor * spec * fragMetallic;

    return (lightColor * diffuse + specular) * attenuation * radiusAtten * spotIntensity;
}

void main()
{
    vec3 tangent = normalize(fragTangent);
    vec3 bitangent = normalize(fragBitangent);
    vec3 N = normalize(fragNormal);
    mat3 TBN = mat3(tangent, bitangent, N);

    vec3 viewDirTS = normalize(transpose(TBN) * (perFrame.cameraPos - fragWorldPos));

    vec2 finalUV = fragUVAtlas;
    if (perObject.parallaxMode == 0)
        finalUV = ParallaxOcclusionMapping(fragUVAtlas, viewDirTS, perObject.parallaxScale);
    else if (perObject.parallaxMode == 1)
        finalUV = ReliefMapping(fragUVAtlas, viewDirTS, perObject.parallaxScale);

    vec4 texColor = texture(texSampler, finalUV);
    vec3 albedo = texColor.rgb;
    if (length(albedo) < 0.001)
        albedo = fragAlbedo;

    float alpha = 1.0;
    if (perObject.alphaMode == 1)
    {
        if (texColor.a < perObject.alphaCutoff) discard;
    }
    else if (perObject.alphaMode == 2)
    {
        alpha = texColor.a > 0.0 ? texColor.a : 0.5;
    }

    vec3 normalMap = texture(texNormalSampler, finalUV).rgb;
    vec3 normal = normalize(TBN * (normalMap * 2.0 - 1.0));

    vec3 viewDir = normalize(perFrame.cameraPos - fragWorldPos);

    vec3 ambient = albedo * 0.3 * fragAO;
    vec3 lighting = vec3(0.0);

    ivec2 screenSize = textureSize(depthSampler, 0);
    vec2 screenPos = gl_FragCoord.xy / vec2(screenSize);

    uint tileX = uint(screenPos.x * float(gridInfo.tileCountX));
    uint tileY = uint(screenPos.y * float(gridInfo.tileCountY));

    float deviceDepth = texture(depthSampler, screenPos).r;
    float linearDepth = (2.0 * perFrame.nearPlane * perFrame.farPlane) /
        (perFrame.farPlane + perFrame.nearPlane - deviceDepth * (perFrame.farPlane - perFrame.nearPlane));

    float depthRange = perFrame.farPlane - perFrame.nearPlane;
    float depthSlice_f = log2(1.0 + (linearDepth - perFrame.nearPlane) / depthRange * 63.0) / log2(64.0) * float(gridInfo.depthSlices);
    uint slice = min(uint(depthSlice_f), gridInfo.depthSlices - 1);

    uint clusterId = slice * gridInfo.tileCountX * gridInfo.tileCountY + tileY * gridInfo.tileCountX + tileX;

    uint clusterLightCount = clusterCount.counts[clusterId];
    uint startIdx = clusterId * 64;

    for (uint i = 0; i < clusterLightCount; i++)
    {
        uint lightIdx = clusterIndex.indices[startIdx + i];
        Light light = lightData.lights[lightIdx];
        int type = int(light.position.w);

        if (type == 0)
            lighting += ComputeDirectionalLight(light, normal, viewDir, albedo);
        else if (type == 1)
            lighting += ComputePointLight(light, fragWorldPos, normal, viewDir, albedo);
        else if (type == 2)
            lighting += ComputeSpotLight(light, fragWorldPos, normal, viewDir, albedo);
    }

    vec3 color = ambient + lighting;
    outColor = vec4(color, alpha);
}
