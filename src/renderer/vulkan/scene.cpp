#include "renderer/vulkan/scene.h"

#include <algorithm>

#include <renderer/vulkan/light/directional.h>
#include <renderer/vulkan/light/point.h>
#include <renderer/vulkan/light/spot.h>

Scene::Scene() {}

Scene::~Scene() 
{
    Destroy();
}

void Scene::Init() {}

void Scene::Update(float deltaTime, class ResourceManager* resourceManager) 
{
    std::vector<LightUBO> lightData;
    for (Light* light : m_lights)
    {
        if (light)
        {
            lightData.push_back(PackLight(light));
        }
    }
    resourceManager->UpdateLightBuffer(lightData, static_cast<int>(lightData.size()));
}

void Scene::Destroy() 
{
    for (Object* obj : m_objects)
        delete obj;
    m_objects.clear();
    for (Light* light : m_lights)
        delete light;
    m_lights.clear();
}

void Scene::AddObject(Object* object) 
{
    if (object)
    {
        m_objects.push_back(object);
    }
}

void Scene::RemoveObject(Object* object) 
{
    auto it = std::find(m_objects.begin(), m_objects.end(), object);
    if (it != m_objects.end())
    {
        m_objects.erase(it);
    }
}

void Scene::AddLight(Light* light)
{
    if (light)
    {
        m_lights.push_back(light);
    }
}

void Scene::RemoveLight(Light* light)
{
    auto it = std::find(m_lights.begin(), m_lights.end(), light);
    if (it != m_lights.end())
    {
        m_lights.erase(it);
    }
}

Camera* Scene::GetCamera() 
{
    return &m_camera;
}

const std::vector<Object*>& Scene::GetObjects() const 
{
    return m_objects;
}

const std::vector<Light*>& Scene::GetLights() const
{
    return m_lights;
}

LightUBO Scene::PackLight(const Light* light) const
{
    LightUBO ubo{};
    LightType type = light->GetType();

    glm::vec3 color = light->GetColor();
    float intensity = light->GetIntensity();
    ubo.color = glm::vec4(color, light->GetCastShadows() ? 1.0f : 0.0f);

    switch (type)
    {
    case LightType::Directional:
    {
        const DirectionalLight* dl = static_cast<const DirectionalLight*>(light);
        ubo.position = glm::vec4(0.0f, 0.0f, 0.0f, static_cast<float>(LightType::Directional));
        ubo.direction = glm::vec4(dl->GetDirection(), intensity);
        ubo.params = glm::vec4(0.0f);
        ubo.atten = glm::vec4(0.0f);
        break;
    }
    case LightType::Point:
    {
        const PointLight* pl = static_cast<const PointLight*>(light);
        ubo.position = glm::vec4(pl->GetPosition(), static_cast<float>(LightType::Point));
        ubo.direction = glm::vec4(0.0f, 0.0f, 0.0f, intensity);
        ubo.params = glm::vec4(0.0f, 0.0f, pl->GetRadius(), 0.0f);
        ubo.atten = glm::vec4(pl->GetConstant(), pl->GetLinear(), pl->GetQuadratic(), 0.0f);
        break;
    }
    case LightType::Spot:
    {
        const SpotLight* sl = static_cast<const SpotLight*>(light);
        ubo.position = glm::vec4(sl->GetPosition(), static_cast<float>(LightType::Spot));
        ubo.direction = glm::vec4(sl->GetDirection(), intensity);
        ubo.params = glm::vec4(cos(sl->GetInnerCutoff()), cos(sl->GetOuterCutoff()), sl->GetRadius(), 0.0f);
        ubo.atten = glm::vec4(sl->GetConstant(), sl->GetLinear(), sl->GetQuadratic(), 0.0f);
        break;
    }
    }

    return ubo;
}