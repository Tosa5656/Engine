#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/mesh.h>

class ObjParser
{
public:
    static bool Load(const std::string& filename, std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open OBJ file: " << filename << std::endl;
            return false;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v")
            {
                glm::vec3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            }
            else if (prefix == "vt")
            {
                glm::vec2 tex;
                iss >> tex.x >> tex.y;
                texCoords.push_back(tex);
            }
            else if (prefix == "vn")
            {
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (prefix == "f")
            {
                std::vector<uint32_t> facePos;
                std::vector<uint32_t> faceTex;
                std::vector<uint32_t> faceNormal;

                std::string vertex;
                while (iss >> vertex)
                {
                    std::istringstream viss(vertex);
                    uint32_t posIdx = 0, texIdx = 0, normalIdx = 0;
                    char slash1, slash2;

                    size_t slashCount = std::count(vertex.begin(), vertex.end(), '/');

                    if (slashCount == 2)
                    {
                        if (viss >> posIdx >> slash1 >> texIdx >> slash2 >> normalIdx)
                        {
                            facePos.push_back(posIdx);
                            faceTex.push_back(texIdx);
                            faceNormal.push_back(normalIdx);
                        }
                    }
                    else if (slashCount == 1)
                    {
                        if (viss >> posIdx >> slash1 >> texIdx)
                        {
                            facePos.push_back(posIdx);
                            faceTex.push_back(texIdx);
                        }
                    }
                    else
                    {
                        viss >> posIdx;
                        facePos.push_back(posIdx);
                    }
                }

                if (facePos.size() >= 3)
                {
                    for (size_t i = 1; i < facePos.size() - 1; ++i)
                    {
                        uint32_t i0 = facePos[0];
                        uint32_t i1 = facePos[i];
                        uint32_t i2 = facePos[i + 1];

                        size_t baseIdx = vertices.size();
                        vertices.push_back(CreateVertex(positions, texCoords, normals, i0, faceTex.size() > 0 ? faceTex[0] : 0, faceNormal.size() > 0 ? faceNormal[0] : 0, baseIdx));
                        vertices.push_back(CreateVertex(positions, texCoords, normals, i1, faceTex.size() > i ? faceTex[i] : 0, faceNormal.size() > i ? faceNormal[i] : 0, baseIdx + 1));
                        vertices.push_back(CreateVertex(positions, texCoords, normals, i2, faceTex.size() > i + 1 ? faceTex[i + 1] : 0, faceNormal.size() > i + 1 ? faceNormal[i + 1] : 0, baseIdx + 2));

                        indices.push_back({static_cast<uint16_t>(baseIdx)});
                        indices.push_back({static_cast<uint16_t>(baseIdx + 1)});
                        indices.push_back({static_cast<uint16_t>(baseIdx + 2)});
                    }
                }
            }
        }

        file.close();

        if (vertices.empty())
        {
            std::cerr << "No vertices loaded from OBJ: " << filename << std::endl;
            return false;
        }

        return true;
    }

private:
    static MeshVertex CreateVertex(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords, const std::vector<glm::vec3>& normals, uint32_t posIdx, uint32_t texIdx, uint32_t normalIdx, size_t vertexCount)
    {
        MeshVertex vertex;

        int32_t pIdx = static_cast<int32_t>(posIdx) - 1;
        if (pIdx >= 0 && static_cast<size_t>(pIdx) < positions.size())
        {
            vertex.pos = positions[pIdx];
        }
        else
        {
            vertex.pos = glm::vec3(0.0f);
        }

        int32_t nIdx = static_cast<int32_t>(normalIdx) - 1;
        if (nIdx >= 0 && static_cast<size_t>(nIdx) < normals.size())
        {
            vertex.normal = normals[nIdx];
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        int32_t tIdx = static_cast<int32_t>(texIdx) - 1;
        if (tIdx >= 0 && static_cast<size_t>(tIdx) < texCoords.size())
        {
            vertex.uv = texCoords[tIdx];
        }
        else
        {
            vertex.uv = glm::vec2(0.0f);
        }

        return vertex;
    }
};