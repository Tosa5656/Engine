#include "renderer/vulkan/scene.h"

#include <algorithm>

Scene::Scene() {}

Scene::~Scene() 
{
    Destroy();
}

void Scene::Init() {}

void Scene::Update(float deltaTime, class ResourceManager* resourceManager) 
{
    for (Object* obj : m_objects)
    {
        if (obj && obj->IsActive())
        {
            obj->UpdateUBO(resourceManager);
        }
    }
}

void Scene::Destroy() 
{
    m_objects.clear();
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

Camera* Scene::GetCamera() 
{
    return &m_camera;
}

std::vector<Object*> Scene::GetObjects() 
{
    return m_objects;
}