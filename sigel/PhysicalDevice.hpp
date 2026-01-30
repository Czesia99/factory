#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class PhysicalDevice
    {
        public:
            vk::raii::PhysicalDevice physicalDevice = nullptr;
            vk::raii::PhysicalDevice& get() { return physicalDevice; }
        private:
            std::vector<const char*> deviceExtensions = {
                vk::KHRSwapchainExtensionName
            };

        public:
            PhysicalDevice() = default;
            void pickPhysicalDevice(vk::raii::Instance &instance);
            void printDeviceInfo();
        private:
            bool isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice);
            uint32_t findQueueFamilies(vk::raii::PhysicalDevice physicalDevice);

    };
}