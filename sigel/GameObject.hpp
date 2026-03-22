#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "Vertex.hpp"
#include "Device.hpp"

namespace sigel
{
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct Buffer
    {
        vk::raii::Buffer buffer = nullptr;
        vk::raii::DeviceMemory memory = nullptr;
        void *mapped = nullptr;
    };

    struct Mesh 
    {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        uint32_t indexCount = 0;
    };

    struct GameObject
    {
        Mesh mesh;
        std::vector<Buffer> uniformBuffers;
        std::vector<vk::raii::DescriptorSet> descriptorSets;
    };

    Buffer createVertexBuffer2(const std::vector<Vertex> &vertices, vk::raii::CommandPool &pool, Device *device);
    void createIndexBuffer2(Buffer &indexBuffer, std::vector<uint32_t> indices, vk::raii::CommandPool &pool, Device *device);
    void createUniformBuffers2(std::vector<Buffer> &uniformBuffers, Device *device);
}