#pragma once

#include <vector>
#include <algorithm>

#include <renderer/vulkan/camera.h>
#include <renderer/vulkan/object.h>

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

    Camera* GetCamera();
    std::vector<Object*> GetObjects();
private:
    Camera m_camera;
    std::vector<Object*> m_objects;
};