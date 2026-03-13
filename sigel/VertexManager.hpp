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

    class VertexManager
    {
        public:
            vk::raii::Buffer vertexBuffer = nullptr;
            vk::raii::DeviceMemory vertexBufferMemory = nullptr;
            vk::raii::Buffer indexBuffer = nullptr;
            vk::raii::DeviceMemory indexBufferMemory = nullptr;

            std::vector<vk::raii::Buffer> uniformBuffers;
            std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;


        private:
            Device *_device = nullptr;

        public:
            void init(Device *device);
            void createVertexBuffer(std::vector<Vertex> vertices, vk::raii::CommandPool& pool);
            void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);
            void createIndexBuffer(std::vector<uint16_t> indices, vk::raii::CommandPool &pool);
            void createUniformBuffers();
            void updateUniformBuffer(uint32_t currentImage);
        private:
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
            void copyBuffer(vk::raii::CommandPool &pool, vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
    };
}