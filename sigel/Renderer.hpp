#pragma once

#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"

namespace sigel
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    class Renderer
    {
        public:
            vk::raii::CommandPool commandPool = nullptr;
            std::vector<vk::raii::CommandBuffer> commandBuffers;

            std::vector<vk::raii::Fence> inFlightFences;

            std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
            std::vector<vk::raii::Semaphore> renderFinishedSemaphores;

            uint32_t frameIndex = 0;
            bool framebufferResized = false;


        private:
            LogicalDevice *_lDevice;
            Swapchain *_swapchain;
            Pipeline *_pipeline;

        public:
            Renderer() = default;
            void init(LogicalDevice *lDevice, Swapchain *swapchain, Pipeline *pipeline);
            void drawFrame();
            void createCommandPool();
            // void createCommandBuffer();
            void createCommandBuffers();
            void recordCommandBuffer(uint32_t imageIndex);
            void createSyncObjects();
        private:
            void transition_image_layout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}