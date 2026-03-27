#include "ResourceManager.hpp"
#include "vkapi/GpuAllocator.hpp"
#include "Utils.hpp"

namespace sigel
{
    void ResourceManager::init(VulkanContext *vctx)
    {
        _vctx = vctx;
    }

    const Mesh& ResourceManager::getMesh(uint32_t index)
    {
        return meshes[index];
    }

    uint32_t ResourceManager::loadMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        Mesh mesh;

        _vctx->allocator.uploadVertex(vertices, mesh);
        _vctx->allocator.uploadIndices(indices, mesh);

        mesh.indexCount = static_cast<uint32_t>(indices.size());
        uint32_t id = static_cast<uint32_t>(meshes.size());
        meshes.emplace_back(std::move(mesh));

        return id;
    }

    const vk::raii::ShaderModule &ResourceManager::getShader(uint32_t index)
    {
        return shaders[index];
    }

    uint32_t ResourceManager::loadShader(const std::string &path)
    {
        auto code = readFile(path);

        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(code.data())
        };

        uint32_t id = static_cast<uint32_t>(shaders.size());
        shaders.emplace_back(_vctx->device.logicalDevice, createInfo);
        return id;
    }

    void ResourceManager::unloadShader(uint32_t id)
    {
        shaders[id].clear();
    }

    vk::raii::ShaderModule ResourceManager::createShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
        vk::raii::ShaderModule shaderModule{ _vctx->device.logicalDevice, createInfo };
        return shaderModule;
    }

    Buffer ResourceManager::createUniformBuffer(vk::DeviceSize size)
    {
        return _vctx->allocator.createUniformBuffer(size);
    }

    void ResourceManager::destroyBuffer(Buffer& buffer)
    {
        _vctx->allocator.destroyBuffer(buffer);
    }

    void ResourceManager::cleanup()
    {
        for (auto& mesh : meshes)
        {
            _vctx->allocator.destroyBuffer(mesh.vertexBuffer);
            _vctx->allocator.destroyBuffer(mesh.indexBuffer);
        }
        meshes.clear();
    }
}