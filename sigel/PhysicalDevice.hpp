#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class PhysicalDevice
    {
        public:
            vk::raii::PhysicalDevice physicalDevice = nullptr;
            vk::raii::PhysicalDevice& getDevice() { return physicalDevice; }

        private:
            std::vector<const char*> deviceExtensions = {
                vk::KHRSwapchainExtensionName
            };

        public:
            PhysicalDevice() = default;
            void pickPhysicalDevice(vk::raii::Instance &instance);
            void printDeviceInfo();
            uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);
        private:
            bool isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice);
    };
}