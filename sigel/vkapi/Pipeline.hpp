#pragma once

#include "Swapchain.hpp"
#include "Device.hpp"

#include <unordered_map>

#include <iostream>

namespace sigel
{
    struct PipelineConfig
    {
        std::string name = "default";
        std::string shaderPath = "../sigel/shaders/slang2.spv";
        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullMode = vk::CullModeFlagBits::eFront;
        vk::FrontFace frontFace = vk::FrontFace::eClockwise;
        vk::Format depthFormat = vk::Format::eD32Sfloat;
        bool depthTest = true;
        bool depthWrite = true;
    };

    struct PipelineInstance
    {
        std::string name = "default";
        PipelineConfig config;
        // std::vector<char> shaderCode;
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::PipelineLayout pipelineLayout = nullptr;

        // vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

        vk::raii::DescriptorSetLayout globalDescriptorSetLayout = nullptr;
        vk::raii::DescriptorSetLayout materialDescriptorSetLayout = nullptr;
    };

    class PipelineManager
    {
        public:
            PipelineConfig defaultConfig;
            bool msaaChanged = false;
        private:
            Swapchain *_swapchain = nullptr;
            Device *_device = nullptr;

            uint32_t nextPipelineID = 0;
            std::unordered_map<std::string, uint32_t> nameIndex;
            std::unordered_map<uint32_t, PipelineInstance> pipelines;

        public:
            static PipelineManager& get();

            PipelineManager() = default;
            void init(Swapchain *swapchain, Device *device);

            uint32_t createPipeline(PipelineConfig config = {});
            void recreateAllPipelines();

            vk::raii::DescriptorSetLayout createGlobalDescriptorSetLayout();
            vk::raii::DescriptorSetLayout createMaterialDescriptorSetLayout();


            void listPipelines() {
                std::cout <<"aaaaaaaaaaaaa" <<  nameIndex.size() << std::endl;
            }

            const PipelineInstance &getPipeline(uint32_t id) const;
            const PipelineInstance &getPipelineByName(std::string &name) const;
            const uint32_t getPipelineID(const std::string &name) const;
        private:
    };
}
