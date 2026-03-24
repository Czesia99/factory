#pragma once

#include "VulkanContext.hpp"
#include "GameObject.hpp"
#include <vma/vk_mem_alloc.h>

namespace sigel
{
    class ResourceManager
    {
        private:
            VulkanContext *_vctx = nullptr;
            std::vector<Mesh> meshes;
            vk::raii::CommandPool transferPool = nullptr;
            VmaAllocator allocator = VK_NULL_HANDLE;

        public:
            ResourceManager() = default;
            void init(VulkanContext *vctx);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);

            Buffer createUniformBuffer(vk::DeviceSize size);
            void cleanup();
            void destroyBuffer(Buffer& buffer);
        private:
            Buffer createBuffer(vk::DeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage);
            Buffer createStagingBuffer(vk::DeviceSize size);
            
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, vk::DeviceSize size);
            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
            void immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn);
    };
}