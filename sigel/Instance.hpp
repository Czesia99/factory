#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class Instance
    {
        private:
            vk::raii::Context  context;
            vk::raii::Instance instance = nullptr;
            vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
        public:
            Instance() = default;
            void createInstance();
            void setupDebugMessenger();
        private:
    };
}