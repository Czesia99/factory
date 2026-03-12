#include "VertexManager.hpp"

namespace sigel
{
    void VertexManager::init(Device *device)
    {
        _device = device;
    }

    void VertexManager::createVertexBuffer(std::vector<Vertex> vertices, vk::raii::CommandPool &pool)
    {
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        vk::BufferCreateInfo stagingInfo{ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive };
        vk::raii::Buffer stagingBuffer(_device->logicalDevice, stagingInfo);
        vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfoStaging{  .allocationSize = memRequirementsStaging.size, .memoryTypeIndex = findMemoryType(memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) };
        vk::raii::DeviceMemory stagingBufferMemory(_device->logicalDevice, memoryAllocateInfoStaging);

        stagingBuffer.bindMemory(stagingBufferMemory, 0);
        void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
        memcpy(dataStaging, vertices.data(), stagingInfo.size);
        stagingBufferMemory.unmapMemory();

        vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
        vertexBuffer = vk::raii::Buffer(_device->logicalDevice, bufferInfo);

        vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{  .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
        vertexBufferMemory = vk::raii::DeviceMemory(_device->logicalDevice, memoryAllocateInfo);
        //TODO: vma

        vertexBuffer.bindMemory( *vertexBufferMemory, 0 );

        copyBuffer(pool, stagingBuffer, vertexBuffer, stagingInfo.size);
    }

    void VertexManager::createBuffer(std::vector<Vertex> vertices, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory)
    {
        vk::BufferCreateInfo bufferInfo{ .size = sizeof(vertices[0]) * vertices.size(), .usage = vk::BufferUsageFlagBits::eVertexBuffer, .sharingMode = vk::SharingMode::eExclusive };
        vertexBuffer = vk::raii::Buffer(_device->logicalDevice, bufferInfo);
        vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{.allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)};
        vertexBufferMemory = vk::raii::DeviceMemory( _device->logicalDevice, memoryAllocateInfo );
        vertexBuffer.bindMemory(*vertexBufferMemory, 0);
    }

    uint32_t VertexManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
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

    void VertexManager::copyBuffer(vk::raii::CommandPool &pool, vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
    {
        vk::CommandBufferAllocateInfo allocInfo{ .commandPool = pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
        vk::raii::CommandBuffer commandCopyBuffer = std::move(_device->logicalDevice.allocateCommandBuffers(allocInfo).front());
        commandCopyBuffer.begin(vk::CommandBufferBeginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
        commandCopyBuffer.end();
        _device->graphicsQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer }, nullptr);
        _device->graphicsQueue.waitIdle();
    }
}