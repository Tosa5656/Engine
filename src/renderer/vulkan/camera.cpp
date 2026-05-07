#include "camera.h"

Camera::Camera() : m_target(glm::vec3(0.0f, 0.0f, 0.0f)), m_up(glm::vec3(0.0f, 1.0f, 0.0f)) {}
Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up) : m_transform(position), m_target(target), m_up(up) {}
Camera::~Camera() {}

void Camera::SetPosition(glm::vec3 position)
{
    m_transform.SetPosition(position);
    m_dirty = true;
}

void Camera::SetTarget(glm::vec3 target)
{
    m_target = target;
    m_dirty = true;
}

void Camera::SetUp(glm::vec3 up)
{
    m_up = up;
    m_dirty = true;
}

void Camera::SetFov(float fovDegrees)
{
    m_fov = fovDegrees;
    m_dirty = true;
}

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
    m_dirty = true;
}

void Camera::SetNearFar(float near, float far)
{
    m_near = near;
    m_far = far;
    m_dirty = true;
}

void Camera::Move(glm::vec3 delta)
{
    m_transform.SetPosition(m_transform.GetPosition() + delta);
    m_target += delta;
    m_dirty = true;
}

void Camera::Rotate(float yaw, float pitch)
{
    glm::vec3 forward = m_target - m_transform.GetPosition();
    glm::vec3 right = glm::normalize(glm::cross(forward, m_up));

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, yaw, m_up);
    rotation = glm::rotate(rotation, pitch, right);
    glm::vec3 newForward = glm::vec3(rotation * glm::vec4(forward, 0.0f));
    m_target = m_transform.GetPosition() + newForward;
    m_dirty = true;
}

glm::vec3 Camera::GetPosition() const
{
    return m_transform.GetPosition();
}

glm::vec3 Camera::GetTarget() const
{
    return m_target;
}

glm::vec3 Camera::GetUp() const
{
    return m_up;
}

const glm::mat4 &Camera::GetViewMatrix()
{
    if (m_dirty) UpdateViewMatrix();
    return m_viewMatrix;
}

const glm::mat4 &Camera::GetProjectionMatrix()
{
    if (m_dirty) UpdateProjectionMatrix();
    return m_projectionMatrix;
}

void Camera::MarkDirty()
{
    m_dirty = true;
}

void Camera::UpdateViewMatrix()
{
    m_viewMatrix = glm::lookAt(m_transform.GetPosition(), m_target, m_up);
}

void Camera::UpdateProjectionMatrix()
{
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    m_projectionMatrix[1][1] *= -1;
}
