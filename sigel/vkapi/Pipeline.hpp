#pragma once

#include "Swapchain.hpp"
#include "Device.hpp"

#include <unordered_map>

namespace sigel
{
    struct PipelineConfig
    {
        std::string name = "default";
        std::string shaderPath = "../sigel/shaders/slang2.spv";
        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullMode = vk::CullModeFlagBits::eFront;
        vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
        vk::Format depthFormat = vk::Format::eD32Sfloat;
        bool depthTest = true;
        bool depthWrite = true;
    };

    struct PipelineInstance
    {
        std::string name = "default";
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
        vk::raii::PipelineLayout pipelineLayout = nullptr;
    };

    class PipelineManager
    {
        public:
        private:
            Swapchain *_swapchain = nullptr;
            Device *_device = nullptr;

            std::vector<PipelineInstance> pipelines;
            std::unordered_map<std::string, uint32_t> nameIndex;
        public:
            static PipelineManager& get() {
                static PipelineManager instance;
                return instance;
            }

            PipelineManager(const PipelineManager&) = delete;
            PipelineManager& operator=(const PipelineManager&) = delete;

            PipelineManager() = default;
            void init(Swapchain *swapchain, Device *device);
            uint32_t createPipeline(PipelineConfig config = {});
            vk::raii::DescriptorSetLayout createDescriptorSetLayout();
            PipelineConfig defaultConfig;

            const PipelineInstance &getPipeline(uint32_t id) const;
            const PipelineInstance &getPipelineByName(std::string &name) const;
            const uint32_t getPipelineID(const std::string &name) const;
        private:
    };
}
