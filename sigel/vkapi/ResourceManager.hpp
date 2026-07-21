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
            std::vector<AllocatedImage> textures;
        private:
            GpuAllocator *_allocator;
            Device *_device;
        public:
            static ResourceManager& get() {
                static ResourceManager instance;
                return instance;
            }

            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;

            ResourceManager() = default;

            void init(GpuAllocator *allocator, Device *device);

            const Mesh &getMesh(uint32_t index);
            uint32_t createMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);
            uint32_t createTextureImage(std::string path);

            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);

            void cleanup();
        private:
            vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    };
}
