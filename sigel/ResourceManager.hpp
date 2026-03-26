#pragma once

#include "vkapi/VulkanContext.hpp"
#include "vkapi/GpuAllocator.hpp"
#include "GameObject.hpp"

namespace sigel
{
    class ResourceManager
    {
        public:
            std::vector<Mesh> meshes;
            std::vector<vk::raii::ShaderModule> shaders;
        private:
            VulkanContext *_vctx = nullptr;
            vk::raii::CommandPool transferPool = nullptr;
        public:
            ResourceManager() = default;
            void init(VulkanContext *vctx);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);


            void cleanup();

            vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);
    };
}