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
private:
    Camera m_camera;
    std::vector<Object*> m_objects;
    std::vector<Light*> m_lights;
};