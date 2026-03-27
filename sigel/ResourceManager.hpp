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
            std::vector<vk::raii::ShaderModule> shaders;
            // std::unordered_map<std::string, uint32_t> shaderCache;
        private:
            VulkanContext *_vctx = nullptr;
        public:
            ResourceManager() = default;
            void init(VulkanContext *vctx);

            const Mesh &getMesh(uint32_t index);
            uint32_t loadMesh(const std::vector<Vertex>&, const std::vector<uint32_t>&);

            const vk::raii::ShaderModule &getShader(uint32_t index);
            uint32_t loadShader(const std::string &path);
            void unloadShader(uint32_t id);

            void cleanup();

            vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
            Buffer createUniformBuffer(vk::DeviceSize size);
            void destroyBuffer(Buffer& buffer);
    };
}