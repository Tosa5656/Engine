#pragma once

#include <renderer/vulkan/mesh.h>
#include <utils/vars/transform.h>

class Object
{
public:
    Object();
    Object(Mesh mesh);

    ~Object();

    void Init();
    void Draw();
    void Destroy();

    Mesh* GetMesh();
    Transform* GetTransform();
private:
    Mesh m_mesh;
    Transform m_transform;
};