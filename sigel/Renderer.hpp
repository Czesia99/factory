#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "ResourceManager.hpp"
#include "GameObject.hpp"
#include "frames.h"

namespace sigel
{
    class Renderer
    {
        public:
        bool framebufferResized = false;
        
        private:
        Device *_device = nullptr;
        Swapchain *_swapchain = nullptr;
        Pipeline *_pipeline = nullptr;
        ResourceManager *_resourceManager  = nullptr;
        
        std::vector<GameObject> loadedObjects;

        vk::raii::CommandPool commandPool = nullptr;
        vk::raii::DescriptorPool descriptorPool = nullptr;
        
        
        uint32_t frameIndex = 0;
        std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
        std::vector<vk::raii::Semaphore> renderSemaphores;

        public:
            void init(Device *device, Swapchain *swapchain, Pipeline *pipeline, ResourceManager *resourceManager);
            void drawFrame();
            void createCommandPool();
            void createDescriptorPool();
            void createDescriptorSets();
            void recordCommandBuffer(uint32_t imageIndex);
            void createFrameData();
            void updateUniformBuffer(uint32_t currentImage);
            void createUniformBuffers(std::vector<Buffer> &uniformBuffers);
            FrameData &currentFrame();

            void loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
            void cleanupObjects();
        
        private:
            void waitFence();
            void checkImageResult(vk::Result result);
            void transition_image_layout(vk::raii::CommandBuffer&cmd, uint32_t imageIndex, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}