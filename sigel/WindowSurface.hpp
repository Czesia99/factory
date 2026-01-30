#pragma once

#include <vulkan/vulkan_raii.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace sigel
{
    class WindowSurface
    {
        public:
            vk::raii::SurfaceKHR &getSurface() { return surface; };
        private:
            vk::raii::SurfaceKHR surface = nullptr;
        public:
            void createSurface(vk::raii::Instance &instance, GLFWwindow *window);
        private:
    };
}