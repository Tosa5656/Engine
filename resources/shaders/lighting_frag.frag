#version 450

layout(location = 0) in vec2 fragUV;

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
    float cascadeUVScale0;
    float cascadeUVScale1;
    float cascadeUVScale2;
    int debugCascades;
    float pcssLightSize;
    float pcssBlockerRadius;
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
    mat4 shadowMatrix;
};

layout(set = 2, binding = 0) readonly buffer LightSSBO
{
    Light lights[1024];
    int lightCount;
} lightData;

layout(set = 3, binding = 0) uniform sampler2DArrayShadow shadowMap;
layout(set = 3, binding = 1) uniform sampler2DArray shadowMapDepth;

layout(set = 4, binding = 0) uniform sampler2DArrayShadow spotShadowMap;
layout(set = 4, binding = 1) uniform sampler2DArray spotShadowMapDepth;

layout(set = 5, binding = 0) uniform sampler2DArrayShadow pointShadowMap;
layout(set = 5, binding = 1) readonly buffer PointShadowMatrices
{
    mat4 matrices[];
} pointShadowData;
layout(set = 5, binding = 2) uniform sampler2DArray pointShadowMapDepth;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outEmissive;

const vec2 poissonDisk[16] = vec2[](
    vec2(-0.942, -0.334), vec2(0.942, 0.334),
    vec2(-0.612, -0.791), vec2(0.612, 0.791),
    vec2(-0.205, -0.979), vec2(0.205, 0.979),
    vec2(-0.821,  0.571), vec2(0.821, -0.571),
    vec2(-0.376,  0.927), vec2(0.376, -0.927),
    vec2(-0.998,  0.067), vec2(0.998, -0.067),
    vec2(-0.122,  0.993), vec2(0.122, -0.993),
    vec2(-0.706, -0.708), vec2(0.706, 0.708)
);

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

mat4 getLightMatrix(int cascade)
{
    if (cascade == 0) return perFrame.lightSpaceMatrix;
    return perFrame.lightSpaceMatrices[cascade - 1];
}

float getCascadeUVScale(int cascade)
{
    if (cascade == 0) return perFrame.cascadeUVScale0;
    if (cascade == 1) return perFrame.cascadeUVScale1;
    return perFrame.cascadeUVScale2;
}

vec3 applyNormalOffset(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    float cosTheta = max(dot(normal, lightDir), 0.001);
    float offsetScale = tan(acos(min(cosTheta, 0.999)));
    offsetScale = min(offsetScale, 5.0);
    float offset = offsetScale * perFrame.normalOffsetBias;
    return worldPos + normal * offset;
}

float sampleCascade(sampler2DArrayShadow samp, sampler2DArray depthSamp, vec2 uv, int layer, float ref, vec2 texelSize, float uvScale, vec2 noiseSeed)
{
    vec2 scaledUV = uv * uvScale;
    vec2 scaledTexelSize = texelSize / uvScale;

    if (perFrame.pcfKernel == 1)
    {
        return texture(samp, vec4(scaledUV, layer, ref));
    }
    else if (perFrame.pcfKernel == 3)
    {
        float sum = 0.0;
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++)
                sum += texture(samp, vec4(scaledUV + vec2(x, y) * scaledTexelSize, layer, ref));
        return sum / 9.0;
    }
    else if (perFrame.pcfKernel == 5)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * scaledTexelSize * 2.0;
            sum += texture(samp, vec4(scaledUV + offset, layer, ref));
        }
        return sum / 16.0;
    }
    else if (perFrame.pcfKernel == 7)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int x = -1; x <= 2; x++)
            for (int y = -1; y <= 2; y++)
            {
                vec2 offset = rot * (vec2(x, y) + 0.5) * scaledTexelSize;
                sum += texture(samp, vec4(scaledUV + offset, layer, ref));
            }
        return sum / 16.0;
    }
    else
    {
        float receiverDepth = ref + perFrame.depthBias;
        float searchUVRadius = perFrame.pcssBlockerRadius * scaledTexelSize.x;
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);

        float avgBlockerDepth = 0.0;
        int blockerCount = 0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * searchUVRadius;
            float sampleDepth = texture(depthSamp, vec3(scaledUV + offset, layer)).r;
            if (sampleDepth < receiverDepth - 0.001)
            {
                avgBlockerDepth += sampleDepth;
                blockerCount++;
            }
        }

        float penumbraWidth = 1.0;
        if (blockerCount > 0)
        {
            avgBlockerDepth /= float(blockerCount);
            penumbraWidth = (receiverDepth - avgBlockerDepth) / avgBlockerDepth * perFrame.pcssLightSize;
            penumbraWidth = clamp(penumbraWidth, 0.5, 20.0);
        }

        vec2 filterSize = scaledTexelSize * penumbraWidth;
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * filterSize;
            sum += texture(samp, vec4(scaledUV + offset, layer, ref));
        }
        return sum / 16.0;
    }
}

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDir, out int cascadeIdx)
{
    vec3 posWithOffset = applyNormalOffset(worldPos, normal, lightDir);

    float viewDepth = abs((perFrame.view * vec4(worldPos, 1.0)).z);
    ivec3 texSize3 = textureSize(shadowMap, 0);
    vec2 texelSize = 1.0 / vec2(texSize3.xy);

    cascadeIdx = -1;

    for (int i = 0; i < 3; i++)
    {
        mat4 lightMat = getLightMatrix(i);
        vec4 lsPosOrig = lightMat * vec4(worldPos, 1.0);
        vec3 projOrig = lsPosOrig.xyz * 0.5 + 0.5;

        if (projOrig.x < 0.0 || projOrig.x > 1.0 ||
            projOrig.y < 0.0 || projOrig.y > 1.0 ||
            projOrig.z < 0.0 || projOrig.z > 1.0)
            continue;

        vec4 lsPosOff = lightMat * vec4(posWithOffset, 1.0);
        vec3 proj = lsPosOff.xyz * 0.5 + 0.5;

        proj.xyz = clamp(proj.xyz, 0.0, 1.0);

        float uvScale = getCascadeUVScale(i);
        float ref = proj.z - perFrame.depthBias;
        vec2 noiseSeed = worldPos.zx;
        float shadow = sampleCascade(shadowMap, shadowMapDepth, proj.xy, i, ref, texelSize, uvScale, noiseSeed);

        if (i < 2)
        {
            float nextSplit = (i == 0) ? perFrame.cascadeSplits.x : perFrame.cascadeSplits.y;
            float thisSplit = (i == 0) ? perFrame.nearPlane : perFrame.cascadeSplits.x;
            float blendDist = (nextSplit - thisSplit) * 0.15;
            float blend = smoothstep(nextSplit - blendDist, nextSplit + blendDist, viewDepth);
            blend = clamp(blend, 0.0, 1.0);

            if (blend > 0.001)
            {
                float uvScale2 = getCascadeUVScale(i + 1);
                mat4 lightMat2 = getLightMatrix(i + 1);
                vec4 lsPos2 = lightMat2 * vec4(posWithOffset, 1.0);
                vec3 proj2 = lsPos2.xyz * 0.5 + 0.5;

                if (proj2.x >= 0.0 && proj2.x <= 1.0 &&
                    proj2.y >= 0.0 && proj2.y <= 1.0 &&
                    proj2.z >= 0.0 && proj2.z <= 1.0)
                {
                    float ref2 = proj2.z - perFrame.depthBias;
                    float shadow2 = sampleCascade(shadowMap, shadowMapDepth, proj2.xy, i + 1, ref2, texelSize, uvScale2, noiseSeed);
                    shadow = mix(shadow, shadow2, blend);
                }
            }
        }

        cascadeIdx = i;
        return 1.0 - shadow;
    }

    return 0.0;
}

vec3 ComputeDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, vec3 worldPos)
{
    vec3 lightDir = normalize(light.direction.xyz);
    float intensity = light.direction.w;
    vec3 lightColor = light.color.rgb * intensity;

    vec3 toLight = -lightDir;
    float diff = max(dot(normal, toLight), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(toLight + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * metallic;

    float shadow = 0.0;
    int cascadeIdx = -1;
    if (light.color.w > 0.5)
        shadow = computeShadow(worldPos, normal, lightDir, cascadeIdx);

    vec3 result = (1.0 - shadow) * (lightColor * diffuse + specular);

    if (perFrame.debugCascades != 0)
    {
        float shade = (1.0 - shadow) * 0.5 + 0.5;
        vec3 debugColor;
        if (cascadeIdx == 0)
            debugColor = vec3(1.0, 0.0, 0.0);
        else if (cascadeIdx == 1)
            debugColor = vec3(0.0, 1.0, 0.0);
        else if (cascadeIdx == 2)
            debugColor = vec3(0.0, 0.0, 1.0);
        else
            debugColor = vec3(0.02);
        result = mix(result, result * debugColor, 0.5);
    }

    return result;
}

float computePointShadow(Light light, vec3 fragPos, vec3 normal)
{
    int shadowIdx = int(light.params.w);
    if (shadowIdx < 0) return 1.0;

    vec3 lightPos = light.position.xyz;
    vec3 fromLight = fragPos - lightPos;
    vec3 lightDir = normalize(fromLight);

    vec3 posWithOffset = applyNormalOffset(fragPos, normal, lightDir);

    vec3 fromLightOff = posWithOffset - lightPos;
    vec3 absDir = abs(fromLightOff);

    int face;
    if (absDir.x >= absDir.y && absDir.x >= absDir.z)
        face = (fromLightOff.x >= 0) ? 0 : 1;
    else if (absDir.y >= absDir.x && absDir.y >= absDir.z)
        face = (fromLightOff.y >= 0) ? 2 : 3;
    else
        face = (fromLightOff.z >= 0) ? 4 : 5;

    int matIdx = shadowIdx * 6 + face;
    vec4 lsPos = pointShadowData.matrices[matIdx] * vec4(posWithOffset, 1.0);
    vec3 proj = lsPos.xyz * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z < 0.0 || proj.z > 1.0)
        return 0.0;

    int layer = shadowIdx * 6 + face;
    float ref = proj.z - perFrame.depthBias;
    ivec3 ptSize = textureSize(pointShadowMap, 0);
    vec2 texelSize = 1.0 / vec2(ptSize.xy);
    vec2 noiseSeed = fragPos.xz + float(shadowIdx);

    if (perFrame.pcfKernel == 1)
    {
        float shadow = texture(pointShadowMap, vec4(proj.xy, layer, ref));
        return 1.0 - shadow;
    }
    else if (perFrame.pcfKernel == 3)
    {
        float sum = 0.0;
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++)
                sum += texture(pointShadowMap, vec4(proj.xy + vec2(x, y) * texelSize, layer, ref));
        return 1.0 - sum / 9.0;
    }
    else if (perFrame.pcfKernel == 5)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * texelSize * 2.0;
            sum += texture(pointShadowMap, vec4(proj.xy + offset, layer, ref));
        }
        return 1.0 - sum / 16.0;
    }
    else if (perFrame.pcfKernel == 7)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int x = -1; x <= 2; x++)
            for (int y = -1; y <= 2; y++)
            {
                vec2 offset = rot * (vec2(x, y) + 0.5) * texelSize;
                sum += texture(pointShadowMap, vec4(proj.xy + offset, layer, ref));
            }
        return 1.0 - sum / 16.0;
    }
    else
    {
        float receiverDepth = ref + perFrame.depthBias;
        float searchUVRadius = perFrame.pcssBlockerRadius * texelSize.x;
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);

        float avgBlockerDepth = 0.0;
        int blockerCount = 0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * searchUVRadius;
            float sampleDepth = texture(pointShadowMapDepth, vec3(proj.xy + offset, layer)).r;
            if (sampleDepth < receiverDepth - 0.001)
            {
                avgBlockerDepth += sampleDepth;
                blockerCount++;
            }
        }

        float penumbraWidth = 1.0;
        if (blockerCount > 0)
        {
            avgBlockerDepth /= float(blockerCount);
            penumbraWidth = (receiverDepth - avgBlockerDepth) / avgBlockerDepth * perFrame.pcssLightSize;
            penumbraWidth = clamp(penumbraWidth, 0.5, 20.0);
        }

        vec2 filterSize = texelSize * penumbraWidth;
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * filterSize;
            sum += texture(pointShadowMap, vec4(proj.xy + offset, layer, ref));
        }
        return 1.0 - sum / 16.0;
    }
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

    float shadow = computePointShadow(light, fragPos, normal);

    return (1.0 - shadow) * (lightColor * diffuse + specular) * attenuation * radiusAtten;
}

float computeSpotShadow(Light light, vec3 fragPos, vec3 normal, vec3 lightDir)
{
    int shadowIdx = int(light.params.w);
    if (shadowIdx < 0) return 1.0;

    vec3 posWithOffset = applyNormalOffset(fragPos, normal, lightDir);
    vec4 lsPos = light.shadowMatrix * vec4(posWithOffset, 1.0);
    vec3 proj = lsPos.xyz * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z < 0.0 || proj.z > 1.0)
        return 0.0;

    float ref = proj.z - perFrame.depthBias;
    ivec3 spotSize = textureSize(spotShadowMap, 0);
    vec2 texelSize = 1.0 / vec2(spotSize.xy);
    vec2 noiseSeed = fragPos.xz + float(shadowIdx);

    if (perFrame.pcfKernel == 1)
    {
        float shadow = texture(spotShadowMap, vec4(proj.xy, shadowIdx, ref));
        return 1.0 - shadow;
    }
    else if (perFrame.pcfKernel == 3)
    {
        float sum = 0.0;
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++)
                sum += texture(spotShadowMap, vec4(proj.xy + vec2(x, y) * texelSize, shadowIdx, ref));
        return 1.0 - sum / 9.0;
    }
    else if (perFrame.pcfKernel == 5)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * texelSize * 2.0;
            sum += texture(spotShadowMap, vec4(proj.xy + offset, shadowIdx, ref));
        }
        return 1.0 - sum / 16.0;
    }
    else if (perFrame.pcfKernel == 7)
    {
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        float sum = 0.0;
        for (int x = -1; x <= 2; x++)
            for (int y = -1; y <= 2; y++)
            {
                vec2 offset = rot * (vec2(x, y) + 0.5) * texelSize;
                sum += texture(spotShadowMap, vec4(proj.xy + offset, shadowIdx, ref));
            }
        return 1.0 - sum / 16.0;
    }
    else
    {
        float receiverDepth = ref + perFrame.depthBias;
        float searchUVRadius = perFrame.pcssBlockerRadius * texelSize.x;
        float angle = rand(noiseSeed) * 6.2831853;
        float s = sin(angle);
        float c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);

        float avgBlockerDepth = 0.0;
        int blockerCount = 0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * searchUVRadius;
            float sampleDepth = texture(spotShadowMapDepth, vec3(proj.xy + offset, shadowIdx)).r;
            if (sampleDepth < receiverDepth - 0.001)
            {
                avgBlockerDepth += sampleDepth;
                blockerCount++;
            }
        }

        float penumbraWidth = 1.0;
        if (blockerCount > 0)
        {
            avgBlockerDepth /= float(blockerCount);
            penumbraWidth = (receiverDepth - avgBlockerDepth) / avgBlockerDepth * perFrame.pcssLightSize;
            penumbraWidth = clamp(penumbraWidth, 0.5, 20.0);
        }

        vec2 filterSize = texelSize * penumbraWidth;
        float sum = 0.0;
        for (int i = 0; i < 16; i++)
        {
            vec2 offset = rot * poissonDisk[i] * filterSize;
            sum += texture(spotShadowMap, vec4(proj.xy + offset, shadowIdx, ref));
        }
        return 1.0 - sum / 16.0;
    }
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
    float theta = dot(-lightDir, spotDir);
    float innerCutoff = light.params.x;
    float outerCutoff = light.params.y;
    float epsilon = innerCutoff - outerCutoff;
    float spotIntensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = lightColor * spec * metallic;

    float shadow = computeSpotShadow(light, fragPos, normal, lightDir);

    return (1.0 - shadow) * (lightColor * diffuse + specular) * attenuation * radiusAtten * spotIntensity;
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
            lighting += ComputeDirectionalLight(light, normal, viewDir, albedo, metallic, worldPos);
        else if (type == 1)
            lighting += ComputePointLight(light, worldPos, normal, viewDir, albedo, metallic);
        else if (type == 2)
            lighting += ComputeSpotLight(light, worldPos, normal, viewDir, albedo, metallic);
    }

    outColor = vec4(ambient + lighting, 1.0);
    outEmissive = vec4(albedo * emissive, 1.0);
}
