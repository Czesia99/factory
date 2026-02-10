#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "Vertex.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

namespace sigel
{
    class VertexManager
    {
        public:
            vk::raii::Buffer vertexBuffer = nullptr;
            vk::raii::DeviceMemory vertexBufferMemory = nullptr;

        private:
            LogicalDevice *_lDevice = nullptr;
            PhysicalDevice *_pDevice = nullptr;

        public:
            void init(LogicalDevice *lDevice, PhysicalDevice *pDevice);
            void createVertexBuffer(std::vector<Vertex> vertices);
        private:
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    };
}