#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class Instance
    {
        public:
            vk::raii::Context  context;
            vk::raii::Instance instance = nullptr;
            vk::raii::Instance& get() { return instance; }
            vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
        public:
            Instance() = default;
            void init();
        private:
            void createInstance();
            void setupDebugMessenger();
    };
}