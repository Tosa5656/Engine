#include "transform.h"

Transform::Transform() {}
Transform::~Transform() {}

Transform::Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    m_position = position;
    m_rotation = rotation;
    m_scale = scale;

    UpdateTransformMatrix();
}

void Transform::SetPosition(glm::vec3 position)
{
    m_position = position;

    UpdateTransformMatrix();
}

void Transform::SetRotation(glm::vec3 rotation)
{
    m_rotation = rotation;

    UpdateTransformMatrix();
}

void Transform::SetScale(glm::vec3 scale)
{
    m_scale = scale;
}

void Transform::Move(glm::vec3 direction)
{
    m_position += direction;

    UpdateTransformMatrix();
}

void Transform::Rotate(float rotation, glm::vec3 axis)
{
    m_rotation += rotation;

    m_transformationMatrix = glm::rotate(m_transformationMatrix, glm::radians(rotation), axis);
}

void Transform::Scale(glm::vec3 scale)
{
    m_scale += scale;

    m_transformationMatrix = glm::scale(m_transformationMatrix, scale);
}

glm::vec3 Transform::GetPosition() const
{
    return m_position;
}

glm::vec3 Transform::GetRotation() const
{
    return m_rotation;
}

glm::vec3 Transform::GetScale() const
{
    return m_scale;
}

glm::mat4 Transform::GetTransformationMatrix() const
{
    return m_transformationMatrix;
}

glm::mat4 Transform::GetInverseTransformationMatrix() const
{
    return glm::inverse(m_transformationMatrix);
}

void Transform::UpdateTransformMatrix()
{
    m_transformationMatrix = glm::mat4(1.0f);
    m_transformationMatrix = glm::translate(m_transformationMatrix, m_position);
    m_transformationMatrix = glm::rotate(m_transformationMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_transformationMatrix = glm::rotate(m_transformationMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_transformationMatrix = glm::rotate(m_transformationMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformationMatrix = glm::scale(m_transformationMatrix, m_scale);
}
