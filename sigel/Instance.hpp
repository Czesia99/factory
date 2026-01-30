#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class Instance
    {
        public:
            vk::raii::Context  context;
            vk::raii::Instance instance = nullptr;
            vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

            vk::raii::Context& getContext() { return context; }
            vk::raii::Instance& getInstance() { return instance; }

        public:
            Instance() = default;
            void init();
        private:
            void createInstance();
            void setupDebugMessenger();
    };
}