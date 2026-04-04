#pragma once

#include <vma/include/vk_mem_alloc.h>
#include "Instance.hpp"
#include "Device.hpp"
#include "../GameObject.hpp"

namespace sigel
{
    struct AllocatedImage 
    {
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
    };

    class GpuAllocator
    {
        public:
            void init(Device *device, Instance *instance);
            Buffer createBuffer(vk::DeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage);
            Buffer createStagingBuffer(vk::DeviceSize size);
            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);
            AllocatedImage createDepthImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);            AllocatedImage createImage(Buffer &imgBuffer, uint32_t width, uint32_t height, VkFormat format);
            AllocatedImage createImageTexture(Buffer &imgBuffer, uint32_t width, uint32_t height, VkFormat format);
            void destroyImage(AllocatedImage &image);
            void immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn);
            void uploadVertex(const std::vector<Vertex> &vertices, Mesh &mesh);
            void uploadIndices(const std::vector<uint32_t> &indices, Mesh &mesh);
            void cleanup();
        private:
            Device *_device = nullptr;
            VmaAllocator allocator = VK_NULL_HANDLE;
            vk::raii::CommandPool transferPool = nullptr;
    };
}