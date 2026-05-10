#pragma once

#include <glm/glm.hpp>
#include <renderer/vulkan/light/light.h>

class SpotLight : public Light
{
public:
    SpotLight();

    LightType GetType() const override { return LightType::Spot; }

    glm::vec3 GetPosition() const;
    void SetPosition(glm::vec3 position);

    glm::vec3 GetDirection() const;
    void SetDirection(glm::vec3 direction);

    float GetInnerCutoff() const;
    float GetOuterCutoff() const;
    float GetConstant() const;
    float GetLinear() const;
    float GetQuadratic() const;
    float GetRadius() const;

    void SetInnerCutoff(float innerCutoff);
    void SetOuterCutoff(float outerCutoff);
    void SetConstant(float constant);
    void SetLinear(float linear);
    void SetQuadratic(float quadratic);
    void SetRadius(float radius);
private:
    glm::vec3 m_direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float m_innerCutoff = glm::radians(12.5f);
    float m_outerCutoff = glm::radians(17.5f);
    float m_constant = 1.0f;
    float m_linear = 0.09f;
    float m_quadratic = 0.032f;
    float m_radius = 10.0f;
};
