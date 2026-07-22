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
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},

        // Back
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},

        // Left
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},

        // Right
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},

        // Top
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},

        // Bottom
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
    };

    const std::vector<uint32_t> cube_indices = {
        // Front
        1,  2,  0,    0,  2,  3,
        // Back
        7,  4,  5,    5,  6,  7,
        // Left
        11, 8,  9,    9,  10, 11,
        // Right
        14, 13, 12,   12, 15, 14,
        // Top
        17, 18, 16,   19, 16, 18,
        // Bottom
        20, 23, 22,   22, 21, 20,
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
