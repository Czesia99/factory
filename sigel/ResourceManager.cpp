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

        _allocator->uploadVertex(vertices, mesh);
        _allocator->uploadIndices(indices, mesh);

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