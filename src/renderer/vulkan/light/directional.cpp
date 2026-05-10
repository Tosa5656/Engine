#include <renderer/vulkan/light/directional.h>

DirectionalLight::DirectionalLight() : Light(), m_direction(0.0f, -1.0f, 0.0f) {}

glm::vec3 DirectionalLight::GetDirection() const
{
    return m_direction;
}

void DirectionalLight::SetDirection(glm::vec3 direction)
{
    m_direction = glm::normalize(direction);
}
