#pragma once

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utils/vars/transform.h>

class Camera
{
public:
    Camera();
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up);
    ~Camera();

    void SetPosition(glm::vec3 position);
    void SetTarget(glm::vec3 target);
    void SetUp(glm::vec3 up);
    void SetFov(float fovDegrees);
    void SetAspectRatio(float aspect);
    void SetNearFar(float near, float far);

    void Move(glm::vec3 delta);
    void Rotate(float yaw, float pitch);

    glm::vec3 GetPosition() const;
    glm::vec3 GetTarget() const;
    glm::vec3 GetUp() const;

    float GetNearPlane() const { return m_near; }
    float GetFarPlane() const { return m_far; }

    const glm::mat4& GetViewMatrix();
    const glm::mat4& GetProjectionMatrix();
    std::vector<glm::vec3> GetFrustumCorners() const;

    void MarkDirty();

private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    Transform m_transform = Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    glm::vec3 m_target;
    glm::vec3 m_up;

    float m_fov = 60.0f;
    float m_aspect = 16.0f / 9.0f;
    float m_near = 0.1f;
    float m_far = 2000.0f;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    bool m_dirty = true;
};