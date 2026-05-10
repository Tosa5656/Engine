#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <renderer/vulkan/mesh.h>

void GenerateSphereMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float radius, int segments, int rings);
void GenerateConeMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float height, float radius, int segments);
void GenerateArrowMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float length, float headLength, float headRadius);