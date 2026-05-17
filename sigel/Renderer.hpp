#pragma once

#include "vkapi/Device.hpp"
#include "vkapi/Swapchain.hpp"
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
            std::vector<GameObject> loadedObjects;
        
        private:
            VulkanContext *_vctx = nullptr;
            ResourceManager *_resourceManager = nullptr;
            PipelineManager *_pipelineManager = nullptr;
            

            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::DescriptorPool descriptorPool = nullptr;
            
            uint32_t frameIndex = 0;
            std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
            std::vector<vk::raii::Semaphore> renderSemaphores;

            // vk::raii::DescriptorSet texturesDescriptorSet;
        public:
            void init(VulkanContext *vctx, ResourceManager *resourceManager, PipelineManager *pipelineManager);
            void drawFrame();
            void createCommandPool();
            void createDescriptorPool();
            void createDescriptorSets();
            // void createTexturesDescriptorSet();
            void recordCommandBuffer(uint32_t imageIndex);
            void createFrameData();
            void updateUniformBuffer(uint32_t currentImage);
            void createUniformBuffers(std::vector<Buffer> &uniformBuffers);
            FrameData &currentFrame();

            void loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t pipelineID, uint32_t textureID);
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