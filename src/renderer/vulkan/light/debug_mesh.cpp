#include <renderer/vulkan/light/debug_mesh.h>

#include <cmath>

static constexpr float PI = 3.14159265359f;

static uint16_t AddVertex(std::vector<MeshVertex>& vertices, glm::vec3 pos, glm::vec3 normal, glm::vec2 uv)
{
    MeshVertex v;
    v.pos = pos;
    v.normal = normal;
    v.uv = uv;
    v.tangent = glm::vec3(0.0f);
    uint16_t idx = static_cast<uint16_t>(vertices.size());
    vertices.push_back(v);
    return idx;
}

static void AddTri(std::vector<MeshIndex>& indices, uint16_t a, uint16_t b, uint16_t c)
{
    indices.push_back({a});
    indices.push_back({b});
    indices.push_back({c});
}

void GenerateSphereMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float radius, int segments, int rings)
{
    vertices.clear();
    indices.clear();

    for (int ring = 0; ring <= rings; ring++)
    {
        float phi = PI * static_cast<float>(ring) / static_cast<float>(rings);
        for (int seg = 0; seg <= segments; seg++)
        {
            float theta = 2.0f * PI * static_cast<float>(seg) / static_cast<float>(segments);
            glm::vec3 dir(
                sin(phi) * cos(theta),
                cos(phi),
                sin(phi) * sin(theta)
            );
            glm::vec3 pos = dir * radius;
            glm::vec2 uv(static_cast<float>(seg) / static_cast<float>(segments), static_cast<float>(ring) / static_cast<float>(rings));
            AddVertex(vertices, pos, dir, uv);
        }
    }

    for (int ring = 0; ring < rings; ring++)
    {
        for (int seg = 0; seg < segments; seg++)
        {
            uint16_t a = static_cast<uint16_t>(ring * (segments + 1) + seg);
            uint16_t b = static_cast<uint16_t>(a + segments + 1);
            uint16_t c = static_cast<uint16_t>(b + 1);
            uint16_t d = static_cast<uint16_t>(a + 1);

            AddTri(indices, a, b, c);
            AddTri(indices, a, c, d);
        }
    }
}

void GenerateConeMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float height, float radius, int segments)
{
    vertices.clear();
    indices.clear();

    uint16_t apex = AddVertex(vertices, glm::vec3(0.0f, height, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 1.0f));
    uint16_t center = AddVertex(vertices, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.0f));

    for (int i = 0; i <= segments; i++)
    {
        float theta = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        glm::vec3 pos(x, 0.0f, z);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
        glm::vec2 uv(static_cast<float>(i) / static_cast<float>(segments), 0.0f);
        uint16_t idx = AddVertex(vertices, pos, normal, uv);
        if (i < segments)
        {
            uint16_t next = static_cast<uint16_t>(idx + 1);
            if (next >= static_cast<uint16_t>(segments + 1)) next = static_cast<uint16_t>(2 + segments);
            AddTri(indices, apex, idx, next);
            AddTri(indices, center, next, idx);
        }
    }
    uint16_t lastRing = static_cast<uint16_t>(2 + segments);
    AddTri(indices, apex, lastRing, 2);
    AddTri(indices, center, 2, lastRing);
}

void GenerateArrowMesh(std::vector<MeshVertex>& vertices, std::vector<MeshIndex>& indices, float length, float headLength, float headRadius)
{
    vertices.clear();
    indices.clear();

    float shaftLength = length - headLength;
    float shaftRadius = headRadius * 0.3f;
    int segments = 16;

    auto addDisk = [&](float y, float r, bool cap) -> uint16_t
    {
        AddVertex(vertices, glm::vec3(0.0f, y, 0.0f), cap ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f));
        for (int i = 0; i < segments; i++)
        {
            float theta = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
            float x = r * cos(theta);
            float z = r * sin(theta);
            glm::vec3 n = cap ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(x, 0.0f, z);
            AddVertex(vertices, glm::vec3(x, y, z), glm::normalize(n), glm::vec2(static_cast<float>(i) / static_cast<float>(segments), 0.0f));
        }
        return static_cast<uint16_t>(vertices.size() - segments - 1);
    };

    auto addBody = [&](float y0, float r0, float y1, float r1)
    {
        for (int i = 0; i < segments; i++)
        {
            float t0 = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
            float t1 = 2.0f * PI * static_cast<float>(i + 1) / static_cast<float>(segments);
            float x0 = r0 * cos(t0), z0 = r0 * sin(t0);
            float x1 = r1 * cos(t1), z1 = r1 * sin(t1);

            uint16_t a = static_cast<uint16_t>(vertices.size());
            AddVertex(vertices, glm::vec3(x0, y0, z0), glm::normalize(glm::vec3(x0, 0.0f, z0)), glm::vec2(static_cast<float>(i) / static_cast<float>(segments), 0.0f));
            uint16_t b = static_cast<uint16_t>(vertices.size());
            AddVertex(vertices, glm::vec3(x1, y0, z1), glm::normalize(glm::vec3(x1, 0.0f, z1)), glm::vec2(static_cast<float>(i + 1) / static_cast<float>(segments), 0.0f));
            uint16_t c = static_cast<uint16_t>(vertices.size());
            AddVertex(vertices, glm::vec3(x1, y1, z1), glm::normalize(glm::vec3(x1, 0.0f, z1)), glm::vec2(static_cast<float>(i + 1) / static_cast<float>(segments), 1.0f));
            uint16_t d = static_cast<uint16_t>(vertices.size());
            AddVertex(vertices, glm::vec3(x0, y1, z0), glm::normalize(glm::vec3(x0, 0.0f, z0)), glm::vec2(static_cast<float>(i) / static_cast<float>(segments), 1.0f));

            AddTri(indices, a, b, c);
            AddTri(indices, a, c, d);
        }
    };

    addBody(0.0f, shaftRadius, shaftLength, shaftRadius);
    addDisk(shaftLength, shaftRadius, true);
    addBody(shaftLength, shaftRadius, length, headRadius);
    addDisk(length, headRadius, true);
}
