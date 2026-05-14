#include <renderer/vulkan/light/point.h>

PointLight::PointLight() : Light()
{
}

glm::vec3 PointLight::GetPosition() const
{
    return m_transform.GetPosition();
}

void PointLight::SetPosition(glm::vec3 position)
{
    m_transform.SetPosition(position);
}

float PointLight::GetConstant() const
{
    return m_constant;
}

float PointLight::GetLinear() const
{
    return m_linear;
}

float PointLight::GetQuadratic() const
{
    return m_quadratic;
}

float PointLight::GetRadius() const
{
    return m_radius;
}

void PointLight::SetConstant(float constant)
{
    m_constant = constant;
}

void PointLight::SetLinear(float linear)
{
    m_linear = linear;
}

void PointLight::SetQuadratic(float quadratic)
{
    m_quadratic = quadratic;
}

void PointLight::SetRadius(float radius)
{
    m_radius = radius;
}
