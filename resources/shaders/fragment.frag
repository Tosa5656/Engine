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

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 3, binding = 0) uniform sampler2D texNormalSampler;
layout(set = 4, binding = 0) uniform sampler2D texHeightSampler;

layout(location = 0) out vec4 outColor;

const float PARALLAX_SCALE = 0.1;
const int PARALLAX_LAYERS = 32;

vec2 ParallaxOcclusionMapping(vec2 texCoord, vec3 viewDirTS, float scale)
{
    float layerDepth = 1.0 / float(PARALLAX_LAYERS);
    float currentLayerDepth = 0.0;
    
    vec2 P = viewDirTS.xy * scale;
    vec2 deltaTexCoords = P / float(PARALLAX_LAYERS);
    
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

void main()
{
    vec3 tangent = normalize(fragTangent);
    vec3 bitangent = normalize(fragBitangent);
    vec3 N = normalize(fragNormal);
    mat3 TBN = mat3(tangent, bitangent, N);
    
    vec3 viewDirTS = normalize(transpose(TBN) * (perFrame.cameraPos - fragWorldPos));
    
    vec2 finalUV = ParallaxOcclusionMapping(fragUVAtlas, viewDirTS, PARALLAX_SCALE);
    
    vec3 albedo = texture(texSampler, finalUV).rgb;
    if (length(albedo) < 0.001)
        albedo = fragAlbedo;

    vec3 normalMap = texture(texNormalSampler, finalUV).rgb;
    vec3 normal = normalize(TBN * (normalMap * 2.0 - 1.0));

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 viewDir = normalize(perFrame.cameraPos - fragWorldPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = albedo * 0.3 * fragAO;
    vec3 diffuse = albedo * diff;

    float fresnel = fragMetallic * pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    vec3 metallicReflect = albedo * fragMetallic * fresnel;

    vec3 color = ambient + diffuse + metallicReflect;
    color = color / (color + vec3(1.0));
    outColor = vec4(color, 1.0);
}