#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
    Transform();
    Transform(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f));

    ~Transform();

    void SetPosition(glm::vec3 position);
    void SetRotation(glm::vec3 rotation);
    void SetScale(glm::vec3 scale);

    void Move(glm::vec3 direction);
    void Rotate(float rotation, glm::vec3 axis);
    void Scale(glm::vec3 scale);

    glm::vec3 GetPosition() const;
    glm::vec3 GetRotation() const;
    glm::vec3 GetScale() const;

    glm::mat4 GetTransformationMatrix() const;
    glm::mat4 GetInverseTransformationMatrix() const;
private:
    void UpdateTransformMatrix();

    glm::mat4 m_transformationMatrix;

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
};