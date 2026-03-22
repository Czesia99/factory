#include "ResourceManager.hpp"

namespace sigel
{
    void ResourceManager::init(Device *device)
    {
        _device = device;

        vk::CommandPoolCreateInfo poolInfo{
            .flags            = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = _device->graphicsIndex
        };
        _transferPool = vk::raii::CommandPool(_device->logicalDevice, poolInfo);
    }

    const Mesh& ResourceManager::getMesh(uint32_t index)
    {
        return meshes[index];
    }

    uint32_t ResourceManager::loadMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        Mesh mesh;
 
        // vertex
        vk::DeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();
        Buffer staging = createBuffer(
            vertexSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        void* data = staging.memory.mapMemory(0, vertexSize);
        memcpy(data, vertices.data(), vertexSize);
        staging.memory.unmapMemory();
 
        mesh.vertexBuffer = createBuffer(
            vertexSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );
        copyBuffer(staging.buffer, mesh.vertexBuffer.buffer, vertexSize);
 
        // index
        vk::DeviceSize indexSize = sizeof(indices[0]) * indices.size();
        Buffer stagingIdx = createBuffer(
            indexSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        void* idxData = stagingIdx.memory.mapMemory(0, indexSize);
        memcpy(idxData, indices.data(), indexSize);
        stagingIdx.memory.unmapMemory();
 
        mesh.indexBuffer = createBuffer(
            indexSize,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(stagingIdx.buffer, mesh.indexBuffer.buffer, indexSize);
 
        mesh.indexCount = static_cast<uint32_t>(indices.size());
        
        uint32_t id = static_cast<uint32_t>(meshes.size());
        meshes.emplace_back(std::move(mesh));
        return id;
    }

    Buffer ResourceManager::createUniformBuffer(vk::DeviceSize size)
    {
        Buffer buffer = createBuffer(
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        buffer.mapped = buffer.memory.mapMemory(0, size);
        return buffer;
    }

    Buffer ResourceManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        Buffer result;
 
        vk::BufferCreateInfo bufferInfo{
            .size        = size,
            .usage       = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };
        result.buffer = vk::raii::Buffer(_device->logicalDevice, bufferInfo);
 
        vk::MemoryRequirements memReqs = result.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize  = memReqs.size,
            .memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties)
        };
        result.memory = vk::raii::DeviceMemory(_device->logicalDevice, allocInfo);
        result.buffer.bindMemory(*result.memory, 0);
 
        return result;
    }

    void ResourceManager::copyBuffer(vk::raii::Buffer &src, vk::raii::Buffer &dst, vk::DeviceSize size)
    {
        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy copyRegion{ .size = size };
            cmd.copyBuffer(*src, *dst, copyRegion);
        });
    }

    uint32_t ResourceManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = _device->physicalDevice.getMemoryProperties();
        

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void ResourceManager::immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn) 
    {
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool        = *_transferPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto cmd = std::move(
            vk::raii::CommandBuffers(_device->logicalDevice, allocInfo).front()
        );
 
        cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        fn(cmd);
        cmd.end();
 
        vk::SubmitInfo submitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers    = &*cmd
        };
        _device->graphicsQueue.submit(submitInfo);
        _device->graphicsQueue.waitIdle(); // safe for uploads — not for frame rendering
    }
}