#pragma once

#include <glm/glm.hpp>
#include <renderer/vulkan/light/light.h>

class PointLight : public Light
{
public:
    PointLight();

    LightType GetType() const override { return LightType::Point; }

    glm::vec3 GetPosition() const;
    void SetPosition(glm::vec3 position);

    float GetConstant() const;
    float GetLinear() const;
    float GetQuadratic() const;
    float GetRadius() const;

    void SetConstant(float constant);
    void SetLinear(float linear);
    void SetQuadratic(float quadratic);
    void SetRadius(float radius);
private:
    float m_constant = 1.0f;
    float m_linear = 0.09f;
    float m_quadratic = 0.032f;
    float m_radius = 10.0f;
};
