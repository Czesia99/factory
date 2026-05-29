#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vma/include/vk_mem_alloc.h>
#include <glm/glm.hpp>
#include "Vertex.hpp"

namespace sigel
{
    struct Buffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        void *mapped = nullptr;
    };

    struct Mesh 
    {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        uint32_t indexCount = 0;
    };
}