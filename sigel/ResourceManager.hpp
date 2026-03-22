#pragma once

#include "GameObject.hpp"

namespace sigel
{
    class ResourceManager
    {
        private:
            Device *_device = nullptr;
            std::vector<Mesh> meshes;
            vk::raii::CommandPool _transferPool = nullptr;

        public:
            ResourceManager() = default;
            void init(Device *device);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);

            Buffer createUniformBuffer(vk::DeviceSize size);
        private:
            Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
            
            void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
            void immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn);
    };
}