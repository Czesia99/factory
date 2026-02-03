#pragma once

#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "WindowSurface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "ShaderManager.hpp"
#include "Renderer.hpp"
#include "Utils.hpp"

namespace sigel
{
    constexpr uint32_t WIDTH = 1920;
    constexpr uint32_t HEIGHT = 1080;

    class SigelEngine {
        public:
            GLFWwindow *window;
            Instance instance;
            WindowSurface surface;
            PhysicalDevice physicalDevice;
            LogicalDevice logicalDevice;
            Swapchain swapchain;
            ShaderManager shaderManager;
            Pipeline pipeline;
            Renderer renderer;
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