#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "ResourceManager.hpp"
#include "frames.h"
#include "../Light.hpp"
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

    struct ObjectPushConstants {
        uint32_t objectIndex;
    };

    struct CameraData {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 pos;
    };

    struct MaterialKey {
        uint32_t pipelineID, diffuse, metallic, roughness, normal;
        bool operator==(const MaterialKey&) const = default;
    };

    struct MaterialKeyHash {
        size_t operator()(const MaterialKey& k) const {
            size_t h = 0;
            for (uint32_t v : { k.pipelineID, k.diffuse, k.metallic, k.roughness, k.normal })
                h ^= std::hash<uint32_t>{}(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    constexpr uint32_t MAX_MATERIAL_SETS = 512;

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

            std::vector<Buffer> uniformBuffers;
            std::vector<Buffer> objectSSBOs;
            std::vector<vk::raii::DescriptorSet> globalDescriptorSets;

            std::unordered_map<MaterialKey, vk::raii::DescriptorSet, MaterialKeyHash> materialSets;
            size_t objectCapacity = 0;

        public:
            void init(Device *device, Swapchain *swapchain, PipelineManager *pipelineManager, ResourceManager *resourceManager);
            void drawFrame(std::span<const RenderItem> items, const CameraData& camera, const DirLight& light, bool showEditor);
            void createCommandPool();
            void createDescriptorPool();
            void recordCommandBuffer(uint32_t imageIndex, std::span<const RenderItem> items, bool showEditor);
            void createFrameData();
            void createUniformBuffers(std::vector<Buffer> &uniformBuffers);
            FrameData &currentFrame();
            float aspectRatio() const;
            void cleanupRenderObjects();

        private:
            void ensureResources(size_t objectCount);
            void writeGlobalDescriptorSets();
            vk::DescriptorSet getMaterialSet(uint32_t pipelineID, const Material& material);
            void updateUniformBuffer(uint32_t currentImage, std::span<const RenderItem> items, const CameraData& camera, const DirLight& light);
            void waitFence();
            void checkImageResult(vk::Result result);
            void transition_image_layout(vk::raii::CommandBuffer&cmd, vk::Image image, vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask);
    };
}
