#include <renderer/vulkan/light/spot.h>

SpotLight::SpotLight() : Light()
{
}

glm::vec3 SpotLight::GetPosition() const
{
    return m_transform.GetPosition();
}

void SpotLight::SetPosition(glm::vec3 position)
{
    m_transform.SetPosition(position);
}

glm::vec3 SpotLight::GetDirection() const
{
    return m_direction;
}

void SpotLight::SetDirection(glm::vec3 direction)
{
    m_direction = glm::normalize(direction);
}

float SpotLight::GetInnerCutoff() const
{
    return m_innerCutoff;
}

float SpotLight::GetOuterCutoff() const
{
    return m_outerCutoff;
}

float SpotLight::GetConstant() const
{
    return m_constant;
}

float SpotLight::GetLinear() const
{
    return m_linear;
}

float SpotLight::GetQuadratic() const
{
    return m_quadratic;
}

float SpotLight::GetRadius() const
{
    return m_radius;
}

void SpotLight::SetInnerCutoff(float innerCutoff)
{
    m_innerCutoff = innerCutoff;
}

void SpotLight::SetOuterCutoff(float outerCutoff)
{
    m_outerCutoff = outerCutoff;
}

void SpotLight::SetConstant(float constant)
{
    m_constant = constant;
}

void SpotLight::SetLinear(float linear)
{
    m_linear = linear;
}

void SpotLight::SetQuadratic(float quadratic)
{
    m_quadratic = quadratic;
}

void SpotLight::SetRadius(float radius)
{
    m_radius = radius;
}
