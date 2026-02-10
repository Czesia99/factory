#include "VertexManager.hpp"

namespace sigel
{
    void VertexManager::init(LogicalDevice *lDevice, PhysicalDevice *pDevice)
    {
        _lDevice = lDevice;
        _pDevice = pDevice;
    }

    void VertexManager::createVertexBuffer(std::vector<Vertex> vertices)
    {
        vk::BufferCreateInfo bufferInfo{ .size = sizeof(vertices[0]) * vertices.size(), .usage = vk::BufferUsageFlagBits::eVertexBuffer, .sharingMode = vk::SharingMode::eExclusive };
        vertexBuffer = vk::raii::Buffer(_lDevice->getDevice(), bufferInfo);
        vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{.allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)};
        vertexBuffer.bindMemory(*vertexBufferMemory, 0);
        void* data = vertexBufferMemory.mapMemory(0, bufferInfo.size);
        memcpy(data, vertices.data(), bufferInfo.size);
        vertexBufferMemory.unmapMemory();
    }

    uint32_t VertexManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = _pDevice->getDevice().getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            {
                return i;
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }
    }
}