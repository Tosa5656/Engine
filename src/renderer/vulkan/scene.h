#pragma once

#include <vector>
#include <algorithm>

#include <renderer/vulkan/camera.h>
#include <renderer/vulkan/object.h>
#include <renderer/vulkan/light/light.h>
#include <renderer/vulkan/resources.h>

class Scene
{
public:
    Scene();
    ~Scene();

    void Init();
    void Update(float deltaTime, class ResourceManager* resourceManager);
    void Destroy();

    void AddObject(Object* object);
    void RemoveObject(Object* object);
    void AddLight(Light* light);
    void RemoveLight(Light* light);

    Camera* GetCamera();
    const std::vector<Object*>& GetObjects() const;
    const std::vector<Light*>& GetLights() const;
    LightUBO PackLight(const Light* light) const;

    int GetSpotShadowCount() const { return m_spotShadowCount; }
    const std::vector<glm::mat4>& GetSpotShadowMatrices() const { return m_spotShadowMatrices; }

    int GetPointShadowCount() const { return m_pointShadowCount; }
    const std::vector<glm::mat4>& GetPointShadowMatrices() const { return m_pointShadowMatrices; }
private:
    Camera m_camera;
    std::vector<Object*> m_objects;
    std::vector<Light*> m_lights;
    int m_spotShadowCount = 0;
    std::vector<glm::mat4> m_spotShadowMatrices;

    int m_pointShadowCount = 0;
    std::vector<glm::mat4> m_pointShadowMatrices;
};