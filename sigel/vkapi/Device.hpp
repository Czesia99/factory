#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class Device {
        public:
            vk::raii::PhysicalDevice physicalDevice = nullptr;
            vk::raii::Device logicalDevice = nullptr;
            vk::raii::Queue graphicsQueue = nullptr;
            vk::raii::Queue presentQueue = nullptr;

            vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

            uint32_t graphicsIndex;
            uint32_t presentIndex;
        private:
            std::vector<const char *> deviceExtensions = {vk::KHRSwapchainExtensionName};

        public:
            Device() = default;
            void pickPhysicalDevice(vk::raii::Instance &instance);
            void createLogicalDevice(vk::raii::SurfaceKHR &surface);
            vk::SampleCountFlagBits getMaxUsableSampleCount();
            void printDeviceInfo();
            uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
        private:

    };
}
