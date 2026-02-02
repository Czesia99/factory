#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "ShaderManager.hpp"

namespace sigel
{
    class Pipeline
    {
        public:
            Pipeline() = default;
            void init(ShaderManager *shaderManager);
        private:
            ShaderManager *_shaderManager;
        public:
            // void createGraphicsPipeline();
            void createGraphicsPipeline(const std::vector<char> &shaderCode);
    };
}