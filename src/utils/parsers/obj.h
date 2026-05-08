#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/mesh.h>

struct MeshVertexHash
{
    size_t operator()(const MeshVertex& vertex) const noexcept
    {
        size_t hash = 17;
        hash = hash * 31 + std::hash<float>{}(vertex.pos.x);
        hash = hash * 31 + std::hash<float>{}(vertex.pos.y);
        hash = hash * 31 + std::hash<float>{}(vertex.pos.z);
        hash = hash * 31 + std::hash<float>{}(vertex.normal.x);
        hash = hash * 31 + std::hash<float>{}(vertex.normal.y);
        hash = hash * 31 + std::hash<float>{}(vertex.normal.z);
        hash = hash * 31 + std::hash<float>{}(vertex.uv.x);
        hash = hash * 31 + std::hash<float>{}(vertex.uv.y);
        hash = hash * 31 + std::hash<float>{}(vertex.tangent.x);
        hash = hash * 31 + std::hash<float>{}(vertex.tangent.y);
        hash = hash * 31 + std::hash<float>{}(vertex.tangent.z);
        return hash;
    }
};

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

        std::unordered_map<MeshVertex, uint32_t, MeshVertexHash> vertexCache;
        vertexCache.reserve(65536);

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
                        std::string first, second;
                        if (viss >> first)
                        {
                            facePos.push_back(std::stoi(first));
                        }
                        if (viss >> slash1 >> second)
                        {
                            if (!second.empty())
                            {
                                faceTex.push_back(std::stoi(second));
                            }
                        }
                    }
                    else if (slashCount == 0)
                    {
                        uint32_t pIdx;
                        viss >> pIdx;
                        facePos.push_back(pIdx);
                    }
                    else
                    {
                        std::string first, second;
                        if (viss >> first)
                        {
                            facePos.push_back(std::stoi(first));
                        }
                        if (viss >> slash1 >> second)
                        {
                            if (!second.empty())
                            {
                                faceTex.push_back(std::stoi(second));
                            }
                        }
                    }
                }

                if (facePos.size() >= 3)
                {
                    for (size_t i = 1; i < facePos.size() - 1; ++i)
                    {
                        uint32_t i0 = facePos[0];
                        uint32_t i1 = facePos[i];
                        uint32_t i2 = facePos[i + 1];

                        uint16_t texIdx0 = faceTex.size() > 0 ? static_cast<uint16_t>(faceTex[0]) : 0;
                        uint16_t texIdx1 = faceTex.size() > i ? static_cast<uint16_t>(faceTex[i]) : 0;
                        uint16_t texIdx2 = faceTex.size() > i + 1 ? static_cast<uint16_t>(faceTex[i + 1]) : 0;

                        uint16_t normIdx0 = faceNormal.size() > 0 ? static_cast<uint16_t>(faceNormal[0]) : 0;
                        uint16_t normIdx1 = faceNormal.size() > i ? static_cast<uint16_t>(faceNormal[i]) : 0;
                        uint16_t normIdx2 = faceNormal.size() > i + 1 ? static_cast<uint16_t>(faceNormal[i + 1]) : 0;

                        MeshVertex v0 = CreateVertex(positions, texCoords, normals, i0, texIdx0, normIdx0);
                        MeshVertex v1 = CreateVertex(positions, texCoords, normals, i1, texIdx1, normIdx1);
                        MeshVertex v2 = CreateVertex(positions, texCoords, normals, i2, texIdx2, normIdx2);

                        auto it0 = vertexCache.find(v0);
                        uint32_t idx0;
                        if (it0 == vertexCache.end())
                        {
                            idx0 = static_cast<uint32_t>(vertices.size());
                            vertexCache.emplace(v0, idx0);
                            vertices.push_back(v0);
                        }
                        else
                        {
                            idx0 = it0->second;
                        }

                        auto it1 = vertexCache.find(v1);
                        uint32_t idx1;
                        if (it1 == vertexCache.end())
                        {
                            idx1 = static_cast<uint32_t>(vertices.size());
                            vertexCache.emplace(v1, idx1);
                            vertices.push_back(v1);
                        }
                        else
                        {
                            idx1 = it1->second;
                        }

                        auto it2 = vertexCache.find(v2);
                        uint32_t idx2;
                        if (it2 == vertexCache.end())
                        {
                            idx2 = static_cast<uint32_t>(vertices.size());
                            vertexCache.emplace(v2, idx2);
                            vertices.push_back(v2);
                        }
                        else
                        {
                            idx2 = it2->second;
                        }

                        indices.push_back({static_cast<uint16_t>(idx0)});
                        indices.push_back({static_cast<uint16_t>(idx1)});
                        indices.push_back({static_cast<uint16_t>(idx2)});
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

        ComputeTangents(vertices, indices);

        return true;
    }

private:
    static void ComputeTangents(std::vector<MeshVertex>& vertices, const std::vector<MeshIndex>& indices)
    {
        std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i].value;
            uint32_t i1 = indices[i + 1].value;
            uint32_t i2 = indices[i + 2].value;

            glm::vec3 pos0 = vertices[i0].pos;
            glm::vec3 pos1 = vertices[i1].pos;
            glm::vec3 pos2 = vertices[i2].pos;

            glm::vec2 uv0 = vertices[i0].uv;
            glm::vec2 uv1 = vertices[i1].uv;
            glm::vec2 uv2 = vertices[i2].uv;

            glm::vec3 deltaPos1 = pos1 - pos0;
            glm::vec3 deltaPos2 = pos2 - pos0;

            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

            glm::vec3 tangent;
            tangent.x = (deltaPos1.x * deltaUV2.y - deltaPos2.x * deltaUV1.y) * r;
            tangent.y = (deltaPos1.y * deltaUV2.y - deltaPos2.y * deltaUV1.y) * r;
            tangent.z = (deltaPos1.z * deltaUV2.y - deltaPos2.z * deltaUV1.y) * r;

            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;
        }

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            glm::vec3 n = vertices[i].normal;
            glm::vec3 t = tangents[i];

            t = glm::normalize(t - n * glm::dot(n, t));

            float w = (glm::dot(glm::cross(n, t), tangents[i]) < 0.0f) ? -1.0f : 1.0f;
            vertices[i].tangent = t * w;
        }
    }

    static MeshVertex CreateVertex(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords, const std::vector<glm::vec3>& normals, uint32_t posIdx, uint32_t texIdx, uint32_t normalIdx)
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