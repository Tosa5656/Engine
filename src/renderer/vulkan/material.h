#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/resources.h>

class Material
{
public:
    Material();
    Material(glm::vec3 albedo, float metallic, float roughness, float ao = 1.0f, float normalStrength = 1.0f);

    void SetAlbedo(glm::vec3 albedo);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetAO(float ao);
    void SetNormalStrength(float normalStrength);

    glm::vec3 GetAlbedo() const;
    float GetMetallic() const;
    float GetRoughness() const;
    float GetAO() const;
    float GetNormalStrength() const;

    const PerObjectUBO& GetData() const;

private:
    PerObjectUBO m_data;
};