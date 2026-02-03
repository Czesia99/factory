#pragma once

#include "LogicalDevice.hpp"

namespace sigel
{
    class Renderer
    {
        public:
            vk::raii::CommandPool commandPool = nullptr;
            vk::raii::CommandBuffer commandBuffer = nullptr;
        private:

        public:
            Renderer() = default;
            void createCommandPool(LogicalDevice &lDevice);
            void createCommandBuffer(LogicalDevice &lDevice);
            // void recordCommandBuffer(uint32_t imageIndex);
        private:
    };
}