#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "ShaderManager.hpp"
#include "Swapchain.hpp"
#include "LogicalDevice.hpp"
#include "VertexManager.hpp"
#include <vector>

namespace sigel
{
    class Pipeline
    {
        public:
            vk::raii::Pipeline graphicsPipeline = nullptr;
            vk::raii::PipelineLayout pipelineLayout = nullptr;
        private:
            Swapchain *_swapchain = nullptr;
            LogicalDevice *_lDevice = nullptr;
        public:
            Pipeline() = default;
            // void createGraphicsPipeline();
            void init(Swapchain *swapchain, LogicalDevice *lDevice);
            void createGraphicsPipeline(vk::raii::ShaderModule &shaderModule);
    };
}