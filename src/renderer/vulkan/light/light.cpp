#include <renderer/vulkan/light/light.h>

Light::Light() : m_transform({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}), m_color(1.0f), m_intensity(1.0f), m_castShadows(false) {}

void Light::SetColor(glm::vec3 color)
{
    m_color = color;
}

void Light::SetIntensity(float intensity)
{
    m_intensity = intensity;
}

void Light::SetCastShadows(bool castShadows)
{
    m_castShadows = castShadows;
}

glm::vec3 Light::GetColor() const
{
    return m_color;
}

float Light::GetIntensity() const
{
    return m_intensity;
}

Transform* Light::GetTransform()
{
    return &m_transform;
}

bool Light::GetCastShadows() const
{
    return m_castShadows;
}
