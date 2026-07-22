#pragma once

#include <vulkan/vulkan_raii.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace sigel
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription() {
            return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
        }

        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
            return {
                vk::VertexInputAttributeDescription({0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)}),
                vk::VertexInputAttributeDescription({1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)})
            };
        }

        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && texCoord == other.texCoord;
        }
    };

    const std::vector<Vertex> cube_vertices = {
        // Front
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
        // Back
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
        {{1.0f,  1.0f, -1.0f},  {1.0f, 1.0f}},
        // Left
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
        // Right
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}},
        // Top
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
        // Bottom
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
    };

    const std::vector<uint32_t> cube_indices = {
        // Front
        0,  2,  1,   3,  2,  0,
        // Back
        5,  4,  7,   7,  6,  5,
        // Left
        9,  8,  11,  11, 10, 9,
        // Right
        12, 13, 14,  14, 15, 12,
        // Top
        16, 18, 17,  18, 16, 19,
        // Bottom
        22, 23, 20,  20, 21, 22,
    };
}

template<>
struct std::hash<sigel::Vertex>
{
    size_t operator()(sigel::Vertex const& vertex) const
    {
        return ((std::hash<glm::vec3>()(vertex.pos)) >> 1) ^ (std::hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
