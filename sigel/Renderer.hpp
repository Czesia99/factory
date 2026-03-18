#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "GameObject.hpp"

namespace sigel
{
    class Renderer
    {
        public:
            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::DescriptorPool descriptorPool = nullptr;
            std::vector<vk::raii::DescriptorSet> descriptorSets;
            std::vector<vk::raii::CommandBuffer> commandBuffers;

            std::vector<vk::raii::Fence> inFlightFences;

            std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
            std::vector<vk::raii::Semaphore> renderFinishedSemaphores;

            uint32_t frameIndex = 0;
            bool framebufferResized = false;

            std::vector<GameObject> loadedObjects;

        private:
            Device *_device = nullptr;
            Swapchain *_swapchain = nullptr;
            Pipeline *_pipeline = nullptr;

        public:
            Renderer() = default;
            void init(Device *device, Swapchain *swapchain, Pipeline *pipeline);
            void drawFrame();
            void createCommandPool();
            void createDescriptorPool();
            void createDescriptorSets();
            // void createCommandBuffer();
            void createCommandBuffers();
            void recordCommandBuffer(uint32_t imageIndex);
            void createSyncObjects();
            void updateUniformBuffer(uint32_t currentImage);

            void loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices); //create GameObject and store in map
        private:
            void transition_image_layout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}