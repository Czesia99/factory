#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

namespace sigel
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;

        static vk::VertexInputBindingDescription getBindingDescription() {
            return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
        }

        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
            return {
                vk::VertexInputAttributeDescription({0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)}),
                vk::VertexInputAttributeDescription({1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)})
            };
        }
    };

    // const std::vector<Vertex> triangle_vertices = {
    //     {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    //     {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    //     {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    //     {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    // };
    

    // const std::vector<uint16_t> triangle_indices = {
    //     0, 1, 2, 2, 3, 0
    // };

    const std::vector<Vertex> cube_vertices = {
        {{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}},

        // Back
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},

        // Left
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},

        // Right
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},

        // Top
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},

        // Bottom
        {{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
    };

    const std::vector<uint32_t> cube_indices = {
        // Front
        0,  1,  2,   2,  3,  0,
        // Back
        5,  4,  7,   7,  6,  5,
        // Left
        9,  8,  11,  11, 10, 9,
        // Right
        12, 13, 14,  14, 15, 12,
        // Top
        16, 17, 18,  18, 19, 16,
        // Bottom
        22, 23, 20,  20, 21, 22,
    };
}