#pragma once

#include <glm/glm.hpp>
#include <renderer/vulkan/light/light.h>

class DirectionalLight : public Light
{
public:
    DirectionalLight();

    LightType GetType() const override { return LightType::Directional; }

    glm::vec3 GetDirection() const;
    void SetDirection(glm::vec3 direction);
private:
    glm::vec3 m_direction = glm::vec3(0.0f, -1.0f, 0.0f);
};
