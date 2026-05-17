#pragma once

#include "vkapi/VulkanContext.hpp"
#include "GameObject.hpp"
#include <unordered_map>

namespace sigel
{
    class ResourceManager
    {
        public:
            std::vector<Mesh> meshes;
            std::vector<AllocatedImage> textures;
        private:
            VulkanContext *_vctx = nullptr;
        public:
            ResourceManager() = default;
            void init(VulkanContext *vctx);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);

            uint32_t createTextureImage(std::string path);
            
            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);
            
            void cleanup();
        private:
            vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    };
}