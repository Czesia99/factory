#pragma once

#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "WindowSurface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"

namespace sigel
{
    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    class SigelEngine {
        public:
            GLFWwindow *window;
            Instance instance;
            WindowSurface surface;
            PhysicalDevice physicalDevice;
            LogicalDevice logicalDevice;
            Swapchain swapchain;
        public:
            SigelEngine() = default;
            void run();
        private:
            void initWindow();
            void initVulkan();
            void mainLoop();
            void cleanup();
    };
}