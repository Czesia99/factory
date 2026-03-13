#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "ShaderManager.hpp"
#include "Swapchain.hpp"
#include "Device.hpp"
#include "VertexManager.hpp"
#include <vector>

namespace sigel
{
    class Pipeline
    {
        public:
            vk::raii::Pipeline graphicsPipeline = nullptr;
            vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
            vk::raii::PipelineLayout pipelineLayout = nullptr;
        private:
            Swapchain *_swapchain = nullptr;
            Device *_device = nullptr;
        public:
            Pipeline() = default;
            // void createGraphicsPipeline();
            void init(Swapchain *swapchain, Device *device);
            void createGraphicsPipeline(vk::raii::ShaderModule &shaderModule);
            void createDescriptorSetLayout();
    };
}