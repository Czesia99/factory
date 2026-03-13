#include "VertexManager.hpp"
#include "frames.h"

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


	void VertexManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory)
	{
		vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
		buffer                                 = vk::raii::Buffer(_device->logicalDevice, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{.allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};
		bufferMemory = vk::raii::DeviceMemory(_device->logicalDevice, allocInfo);
		buffer.bindMemory(bufferMemory, 0);
	}

    void VertexManager::createIndexBuffer(std::vector<uint16_t> indices, vk::raii::CommandPool &pool) 
    {
        vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        vk::raii::Buffer stagingBuffer({});
        vk::raii::DeviceMemory stagingBufferMemory({});
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
        

        void* data = stagingBufferMemory.mapMemory(0, bufferSize);
        memcpy(data, indices.data(), (size_t) bufferSize);
        stagingBufferMemory.unmapMemory();

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

        copyBuffer(pool, stagingBuffer, indexBuffer, bufferSize);
    }

    void VertexManager::createUniformBuffers() 
    {
        uniformBuffers.clear();
        uniformBuffersMemory.clear();
        uniformBuffersMapped.clear();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
            vk::raii::Buffer buffer({});
            vk::raii::DeviceMemory bufferMem({});
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
            uniformBuffers.emplace_back(std::move(buffer));
            uniformBuffersMemory.emplace_back(std::move(bufferMem));
            uniformBuffersMapped.emplace_back( uniformBuffersMemory[i].mapMemory(0, bufferSize));
        }
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