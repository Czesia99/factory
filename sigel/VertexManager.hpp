#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "Vertex.hpp"
#include "Device.hpp"

namespace sigel
{
    class VertexManager
    {
        public:
            vk::raii::Buffer vertexBuffer = nullptr;
            vk::raii::DeviceMemory vertexBufferMemory = nullptr;

        private:
            Device *_device = nullptr;

        public:
            void init(Device *device);
            void createVertexBuffer(std::vector<Vertex> vertices, vk::raii::CommandPool& pool);
            void createBuffer(std::vector<Vertex> vertices, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
        private:
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
            void copyBuffer(vk::raii::CommandPool &pool, vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
    };
}