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

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float padding;
} perFrame;

layout(set = 1, binding = 0) uniform PerObjectUBO
{
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
} perObject;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 0) uniform sampler2D texNormalSampler;
layout(set = 4, binding = 0) uniform sampler2D texHeightSampler;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial;

vec2 ParallaxOcclusionMapping(vec2 texCoord, vec3 viewDirTS, float scale)
{
    int layers = max(perObject.parallaxIterations, 1);
    float layerDepth = 1.0 / float(layers);
    float currentLayerDepth = 0.0;
    vec2 P = viewDirTS.xy * scale;
    vec2 deltaTexCoords = P / float(layers);
    vec2 currentTexCoords = texCoord;
    float currentDepthMapValue = texture(texHeightSampler, currentTexCoords).r;
    while(currentLayerDepth < currentDepthMapValue)
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
    for(int i = 0; i < iterations; i++)
    {
        vec2 sampleCoord = texCoord - ray.xy * depth;
        float h = texture(texHeightSampler, sampleCoord).r;
        if(h > depth) depth += step;
    }
    float binaryStep = step * 0.5;
    for(int i = 0; i < 5; i++)
    {
        vec2 sampleCoord = texCoord - ray.xy * depth;
        float h = texture(texHeightSampler, sampleCoord).r;
        if(h > depth) depth += binaryStep;
        else depth -= binaryStep;
        binaryStep *= 0.5;
    }
    return texCoord - ray.xy * depth;
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

    vec3 albedo = texture(texSampler, finalUV).rgb;
    if (length(albedo) < 0.001)
        albedo = fragAlbedo;

    vec3 normalMap = texture(texNormalSampler, finalUV).rgb;
    vec3 normal = normalize(TBN * (normalMap * 2.0 - 1.0));

    outPosition = vec4(fragWorldPos, 1.0);
    outNormal = vec4(normal, 1.0);
    outAlbedo = vec4(albedo, fragMetallic);
    outMaterial = vec4(fragRoughness, fragAO, 0.0, 1.0);
}
