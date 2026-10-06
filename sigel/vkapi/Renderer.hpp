#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "ResourceManager.hpp"
#include "frames.h"
#include "../Scene.hpp"
#include "../Vertex.hpp"
#include <sigel/ecs/Entity.hpp>
#include <span>
#include <sigel/ecs/systems/RenderItem.hpp>

namespace sigel
{
    struct UniformBufferObject {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) DirLight light;
        alignas(16) glm::vec3 camPos;

    };

    struct ObjBufferObject {
        glm::mat4 model;
    };

    struct MeshRenderData
    {
        uint32_t meshID;
        Material material;
        std::vector<vk::raii::DescriptorSet> descriptorSets;
    };

    struct RenderObject
    {
        uint32_t pipelineID;
        std::vector<MeshRenderData> meshes;
    };

    struct ObjectPushConstants {
        uint32_t objectIndex;
    };

    class Renderer
    {
        public:
            bool framebufferResized = false;

        private:
            Device* _device;
            Swapchain* _swapchain;
            PipelineManager* _pipelineManager;
            ResourceManager* _resourceManager;

            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::DescriptorPool descriptorPool = nullptr;

            uint32_t frameIndex = 0;
            std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
            std::vector<vk::raii::Semaphore> renderSemaphores;

            std::vector<RenderObject> renderObjects;

            std::vector<Buffer> uniformBuffers;
            std::vector<Buffer> objectSSBOs;
            std::vector<vk::raii::DescriptorSet> globalDescriptorSets;

        public:
            void init(Device *device, Swapchain *swapchain, PipelineManager *pipelineManager, ResourceManager *resourceManager);
            void drawFrame(Scene& scene, std::span<const RenderItem> items, bool showEditor);
            void createCommandPool();
            void createDescriptorPool();
            void createDescriptorSets();
            void recordCommandBuffer(uint32_t imageIndex, bool showEditor);
            void createFrameData();
            void updateUniformBuffer(uint32_t currentImage, Scene& scene, std::span<const RenderItem> items);
            void createUniformBuffers(std::vector<Buffer> &uniformBuffers);
            FrameData &currentFrame();

            void prepareScene(std::span<const RenderItem> items);
            void cleanupRenderObjects();

        private:
            void waitFence();
            void checkImageResult(vk::Result result);
            void transition_image_layout(vk::raii::CommandBuffer&cmd, vk::Image image, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}
