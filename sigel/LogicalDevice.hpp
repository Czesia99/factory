#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class LogicalDevice
    {
        public:
            vk::raii::Device device = nullptr;
            vk::raii::Queue graphicsQueue = nullptr;
            vk::raii::Queue presentQueue = nullptr;

            uint32_t graphicsIndex;
            uint32_t presentIndex;

            vk::raii::Device& getDevice() { return device; }
        private:
            std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};
        public:
            void createLogicalDevice(vk::raii::PhysicalDevice &pdevice, vk::raii::SurfaceKHR &surface);
        private:
    };
}