#pragma once

#include <vma/vk_mem_alloc.h>
#include "Instance.hpp"
#include "Device.hpp"
#include "GameObject.hpp"

namespace sigel
{
    struct AllocatedImage 
    {
        VkImage       image      = VK_NULL_HANDLE;
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
            AllocatedImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
            void destroyImage(AllocatedImage& image);
            void immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn);
            void cleanup();
        private:
            Device *_device = nullptr;
            VmaAllocator allocator = VK_NULL_HANDLE;
            vk::raii::CommandPool transferPool = nullptr;
    };
}