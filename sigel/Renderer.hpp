#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "GameObject.hpp"
#include "frames.h"

namespace sigel
{
    class Renderer
    {
        public:
            bool framebufferResized = false;
            std::vector<GameObject> loadedObjects;

        private:
            Device *_device = nullptr;
            Swapchain *_swapchain = nullptr;
            Pipeline *_pipeline = nullptr;

            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::DescriptorPool descriptorPool = nullptr;
            std::vector<vk::raii::DescriptorSet> descriptorSets;

            uint32_t frameIndex = 0;
            std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
            std::vector<vk::raii::Semaphore> renderSemaphores;

        public:
            void init(Device *device, Swapchain *swapchain, Pipeline *pipeline);
            void drawFrame();
            void createCommandPool();
            void createDescriptorPool();
            void createDescriptorSets();
            void recordCommandBuffer(uint32_t imageIndex);
            void createFrameData();
            void updateUniformBuffer(uint32_t currentImage);
            FrameData &currentFrame();

            void loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices); //create GameObject and store in map
        
        private:
            void waitFence();
            void checkImageResult(vk::Result result);
            void transition_image_layout(vk::raii::CommandBuffer&cmd, uint32_t imageIndex, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}