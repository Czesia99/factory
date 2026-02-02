#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "ShaderManager.hpp"

namespace sigel
{
    class Pipeline
    {
        public:
            Pipeline() = default;
        private:
        public:
            // void createGraphicsPipeline();
            void createGraphicsPipeline(vk::raii::ShaderModule &shaderModule);
    };
}