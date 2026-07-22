#include "ResourceManager.hpp"
#include "../Utils.hpp"
#include "../SigelEngine.hpp"

#include <stb_image.h>

namespace sigel
{
    void ResourceManager::init(GpuAllocator *allocator, Device *device)
    {
        _allocator = allocator;
        _device = device;
    }

    ResourceManager& ResourceManager::get()
    {
        return SigelEngine::get().vctx.resourceManager;
    }

    const Mesh& ResourceManager::getMesh(uint32_t index)
    {
        return meshes[index];
    }

    uint32_t ResourceManager::createMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        Mesh mesh;

        _allocator->uploadVertex(vertices, mesh.vertexBuffer);
        _allocator->uploadIndices(indices, mesh.indexBuffer);

        mesh.indexCount = static_cast<uint32_t>(indices.size());
        uint32_t id = static_cast<uint32_t>(meshes.size());
        meshes.emplace_back(std::move(mesh));

        return id;
    }

    uint32_t ResourceManager::createTextureImage(std::string path)
    {
        int width, height, channels;
        stbi_uc *pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = width * height * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        Buffer imgBuffer = _allocator->createStagingBuffer(imageSize);
        memcpy(imgBuffer.mapped, pixels, imageSize);
        stbi_image_free(pixels);

        auto texture = _allocator->createImageTexture(imgBuffer, width, height, VK_FORMAT_R8G8B8A8_SRGB);

        _allocator->destroyBuffer(imgBuffer);
        uint32_t id = static_cast<uint32_t>(textures.size());
        textures.emplace_back(std::move(texture));
        return id;
    }

    vk::raii::ShaderModule ResourceManager::createShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
        vk::raii::ShaderModule shaderModule{ _device->logicalDevice, createInfo };
        return shaderModule;
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
            destroyBuffer(mesh.vertexBuffer);
            destroyBuffer(mesh.indexBuffer);
        }
        meshes.clear();

        for (auto &texture : textures)
        {
            _allocator->destroyImage(texture);
        }
        textures.clear();
    }
}
