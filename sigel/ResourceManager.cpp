#include "ResourceManager.hpp"

namespace sigel
{
    void ResourceManager::init(VulkanContext *vctx)
    {
        _vctx = vctx;

        VmaAllocatorCreateInfo allocatorInfo{
            .physicalDevice = *_vctx->device.physicalDevice,
            .device         = *_vctx->device.logicalDevice,
            .instance       = *_vctx->instance.instance,
        };

    vmaCreateAllocator(&allocatorInfo, &allocator);
        vk::CommandPoolCreateInfo poolInfo{
            .flags            = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = _vctx->device.graphicsIndex
        };
        transferPool = vk::raii::CommandPool(_vctx->device.logicalDevice, poolInfo);
    }

    const Mesh& ResourceManager::getMesh(uint32_t index)
    {
        return meshes[index];
    }

    uint32_t ResourceManager::loadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Mesh mesh;

        vk::DeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingV;
        VkBufferCreateInfo stagingVInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = vertexSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        VmaAllocationCreateInfo stagingAllocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        vmaCreateBuffer(allocator, &stagingVInfo, &stagingAllocInfo, &stagingV.buffer, &stagingV.allocation, nullptr);

        void* vdata;
        vmaMapMemory(allocator, stagingV.allocation, &vdata);
        memcpy(vdata, vertices.data(), vertexSize);
        vmaUnmapMemory(allocator, stagingV.allocation);

        VkBufferCreateInfo vertexBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = vertexSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        };
        VmaAllocationCreateInfo vertexAllocInfo{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
        vmaCreateBuffer(allocator, &vertexBufferInfo, &vertexAllocInfo,
            &mesh.vertexBuffer.buffer, &mesh.vertexBuffer.allocation, nullptr);

        copyBuffer(stagingV.buffer, mesh.vertexBuffer.buffer, vertexSize);
        vmaDestroyBuffer(allocator, stagingV.buffer, stagingV.allocation);

        vk::DeviceSize indexSize = sizeof(indices[0]) * indices.size();

        Buffer stagingBuffer;
        VkBufferCreateInfo stagingInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = indexSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        vmaCreateBuffer(allocator, &stagingInfo, &stagingAllocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr);

        void* idata;
        vmaMapMemory(allocator, stagingBuffer.allocation, &idata);
        memcpy(idata, indices.data(), indexSize);
        vmaUnmapMemory(allocator, stagingBuffer.allocation);

        VkBufferCreateInfo indexBufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = indexSize,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        };
        VmaAllocationCreateInfo indexAllocInfo{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
        vmaCreateBuffer(allocator, &indexBufferInfo, &indexAllocInfo, &mesh.indexBuffer.buffer, &mesh.indexBuffer.allocation, nullptr);

        copyBuffer(stagingBuffer.buffer, mesh.indexBuffer.buffer, indexSize);
        vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);

        mesh.indexCount = static_cast<uint32_t>(indices.size());
        uint32_t id = static_cast<uint32_t>(meshes.size());
        meshes.emplace_back(std::move(mesh));
        return id;
    }

    Buffer ResourceManager::createUniformBuffer(vk::DeviceSize size)
    {
        Buffer result;
        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        VmaAllocationCreateInfo allocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        VmaAllocationInfo info{};
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &info);

        result.mapped = info.pMappedData;
        return result;
    }

    Buffer ResourceManager::createBuffer(vk::DeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage)
    {
        Buffer result;

        VkBufferCreateInfo bufferInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocInfo{
            .usage = memoryUsage
        };

        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, nullptr);

        return result;
    }

    Buffer ResourceManager::createStagingBuffer(vk::DeviceSize size)
    {
        Buffer result;
        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        VmaAllocationCreateInfo allocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, nullptr);
        return result;
    }

    void ResourceManager::destroyBuffer(Buffer& buffer)
    {        
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
        buffer.buffer     = VK_NULL_HANDLE;
        buffer.allocation = VK_NULL_HANDLE;
        buffer.mapped     = nullptr;
    }

    void ResourceManager::copyBuffer(VkBuffer src, VkBuffer dst, vk::DeviceSize size)
    {
        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy copyRegion{ .size = size };
            cmd.copyBuffer(src, dst, copyRegion);
        });
    }

    void ResourceManager::immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn) 
    {
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool        = *transferPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto cmd = std::move(
            vk::raii::CommandBuffers(_vctx->device.logicalDevice, allocInfo).front()
        );
 
        cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        fn(cmd);
        cmd.end();
 
        vk::SubmitInfo submitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers    = &*cmd
        };
        _vctx->device.graphicsQueue.submit(submitInfo);
        _vctx->device.graphicsQueue.waitIdle();
    }

    void ResourceManager::cleanup()
    {
        for (auto& mesh : meshes)
        {
            destroyBuffer(mesh.vertexBuffer);
            destroyBuffer(mesh.indexBuffer);
        }
        meshes.clear();
        vmaDestroyAllocator(allocator);
    }
}