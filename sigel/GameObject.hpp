#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

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

    struct GameObject
    {
        uint32_t meshID;
        std::vector<Buffer> uniformBuffers;
        std::vector<vk::raii::DescriptorSet> descriptorSets;
    };
}