#include "GameObject.hpp"
#include "BufferUtils.hpp"
#include "frames.h"
#include <iostream>

namespace sigel
{
    Buffer createVertexBuffer2(const std::vector<Vertex> &vertices, vk::raii::CommandPool &pool, Device *device)
    {
        Buffer vb{};
        std::cout << "in create vertex buffer2" << std::endl;
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        vk::BufferCreateInfo stagingInfo{ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive };
        vk::raii::Buffer stagingBuffer(device->logicalDevice, stagingInfo);
        vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfoStaging{  .allocationSize = memRequirementsStaging.size, .memoryTypeIndex = findMemoryType(*device, memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) };
        vk::raii::DeviceMemory stagingBufferMemory(device->logicalDevice, memoryAllocateInfoStaging);

        std::cout << "before copy" << std::endl;
        stagingBuffer.bindMemory(stagingBufferMemory, 0);
        void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
        memcpy(dataStaging, vertices.data(), stagingInfo.size);
        stagingBufferMemory.unmapMemory();
        std::cout << "after copy" << std::endl;

        vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
        vb.buffer = vk::raii::Buffer(device->logicalDevice, bufferInfo);

        std::cout << "caca copy" << std::endl;
        vk::MemoryRequirements memRequirements = vb.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{  .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(*device, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
        vb.memory = vk::raii::DeviceMemory(device->logicalDevice, memoryAllocateInfo);
        std::cout << "caca copy 2" << std::endl;
        vb.buffer.bindMemory( *vb.memory, 0 );

        copyBuffer(*device, pool, stagingBuffer, vb.buffer, stagingInfo.size);
        std::cout << "caca copy 3" << std::endl;
        return vb;
    }

    void createIndexBuffer2(Buffer &indexBuffer, std::vector<uint32_t> indices, vk::raii::CommandPool &pool, Device *device)
    {
        vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        vk::raii::Buffer stagingBuffer({});
        vk::raii::DeviceMemory stagingBufferMemory({});
        createBuffer(*device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
        

        void* data = stagingBufferMemory.mapMemory(0, bufferSize);
        memcpy(data, indices.data(), (size_t) bufferSize);
        stagingBufferMemory.unmapMemory();

        createBuffer(*device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer.buffer, indexBuffer.memory);

        copyBuffer(*device, pool, stagingBuffer, indexBuffer.buffer, bufferSize);
    }

    void createUniformBuffers2(std::vector<Buffer> &uniformBuffers, Device *device)
    {
        for (auto &buffer : uniformBuffers)
        {
            buffer.buffer.clear();
            buffer.memory.clear();
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
            vk::raii::Buffer buffer({});
            vk::raii::DeviceMemory bufferMem({});
            createBuffer(*device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);

            Buffer uniformBuffer;
            uniformBuffer.buffer = std::move(buffer);
            uniformBuffer.memory = std::move(bufferMem);
            uniformBuffer.mapped = uniformBuffer.memory.mapMemory(0, bufferSize);

            uniformBuffers.emplace_back(std::move(uniformBuffer));
        }
    }
}