#pragma once

#include "GpuAllocator.hpp"
#include "Device.hpp"
#include "../Mesh.hpp"
#include "../Vertex.hpp"
#include <unordered_map>

namespace sigel
{
    class ResourceManager
    {
        public:
            std::vector<Mesh> meshes;

            std::unordered_map<std::string, uint32_t> texPathIndex;
            std::unordered_map<uint32_t, AllocatedImage> textures;
        private:
            GpuAllocator *_allocator;
            Device *_device;
        public:
            static ResourceManager& get();

            ResourceManager() = default;

            void init(GpuAllocator *allocator, Device *device);

            const Mesh &getMesh(uint32_t index);
            uint32_t createMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);
            uint32_t createTextureImage(std::string path);
            uint32_t createTextureImageFromMemory(const void* buffer, size_t bufferSize, const std::string &path);

            Buffer createUniformBuffer(vk::DeviceSize size);
            Buffer createStorageBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);

            void cleanup();
        private:
            vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    };
}
