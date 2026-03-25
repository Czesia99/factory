#pragma once

#include "VulkanContext.hpp"
#include "GameObject.hpp"
#include "GpuAllocator.hpp"

namespace sigel
{
    class ResourceManager
    {
        public:
            GpuAllocator *_allocator = nullptr;
        private:
            VulkanContext *_vctx = nullptr;
            std::vector<Mesh> meshes;
            vk::raii::CommandPool transferPool = nullptr;

        public:
            ResourceManager() = default;
            void init(GpuAllocator *allocator);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);

            void cleanup();

            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);
        private:
            
    };
}