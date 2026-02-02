#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "ShaderManager.hpp"
#include "Swapchain.hpp"
#include "LogicalDevice.hpp"
#include <vector>

namespace sigel
{
    class Pipeline
    {
        public:
            vk::raii::PipelineLayout pipelineLayout = nullptr;
        private:
            Swapchain *_swapchain;
            LogicalDevice *_lDevice;
        public:
            Pipeline() = default;
            // void createGraphicsPipeline();
            void init(Swapchain *swapchain, LogicalDevice *lDevice);
            void createGraphicsPipeline(vk::raii::ShaderModule &shaderModule);
    };
}