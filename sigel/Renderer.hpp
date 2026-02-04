#pragma once

#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"

namespace sigel
{
    class Renderer
    {
        public:
            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::CommandBuffer commandBuffer = nullptr;
        private:
            LogicalDevice *_lDevice;
            Swapchain *_swapchain;
            Pipeline *_pipeline;

        public:
            Renderer() = default;
            void init(LogicalDevice *lDevice, Swapchain *swapchain, Pipeline *pipeline);
            void createCommandPool();
            void createCommandBuffer();
            void recordCommandBuffer(uint32_t imageIndex);
            void transition_image_layout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
        private:
    };
}