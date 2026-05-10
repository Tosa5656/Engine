#pragma once

#include <glm/glm.hpp>
#include <utils/vars/transform.h>

enum class LightType : uint32_t
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

class Light
{
public:
    Light();
    virtual ~Light() = default;

    virtual LightType GetType() const = 0;

    void SetColor(glm::vec3 color);
    void SetIntensity(float intensity);
    void SetCastShadows(bool castShadows);

    glm::vec3 GetColor() const;
    float GetIntensity() const;
    Transform* GetTransform();
    bool GetCastShadows() const;
protected:
    Transform m_transform;
    glm::vec3 m_color = glm::vec3(1.0f);
    float m_intensity = 1.0f;
    bool m_castShadows = false;
};