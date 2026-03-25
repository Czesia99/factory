#include "ResourceManager.hpp"

namespace sigel
{
    void ResourceManager::init(GpuAllocator *allocator)
    {
        _allocator = allocator;
    }

    const Mesh& ResourceManager::getMesh(uint32_t index)
    {
        return meshes[index];
    }

    uint32_t ResourceManager::loadMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        Mesh mesh;

        vk::DeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingV = _allocator->createStagingBuffer(vertexSize);
        memcpy(stagingV.mapped, vertices.data(), vertexSize);

        mesh.vertexBuffer = _allocator->createBuffer(
            vertexSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        _allocator->immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = vertexSize };
            cmd.copyBuffer(stagingV.buffer, mesh.vertexBuffer.buffer, region);
        });
        _allocator->destroyBuffer(stagingV);

        vk::DeviceSize indexSize = sizeof(indices[0]) * indices.size();

        Buffer stagingI = _allocator->createStagingBuffer(indexSize);
        memcpy(stagingI.mapped, indices.data(), indexSize);

        mesh.indexBuffer = _allocator->createBuffer(
            indexSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        _allocator->immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = indexSize };
            cmd.copyBuffer(stagingI.buffer, mesh.indexBuffer.buffer, region);
        });
        _allocator->destroyBuffer(stagingI);

        mesh.indexCount = static_cast<uint32_t>(indices.size());
        uint32_t id = static_cast<uint32_t>(meshes.size());
        meshes.emplace_back(std::move(mesh));
        return id;
    }

    Buffer ResourceManager::createUniformBuffer(vk::DeviceSize size)
    {
        return _allocator->createUniformBuffer(size);
    }

    void ResourceManager::destroyBuffer(Buffer& buffer)
    {
        _allocator->destroyBuffer(buffer);
    }

    void ResourceManager::cleanup()
    {
        for (auto& mesh : meshes)
        {
            _allocator->destroyBuffer(mesh.vertexBuffer);
            _allocator->destroyBuffer(mesh.indexBuffer);
        }
        meshes.clear();
    }
}