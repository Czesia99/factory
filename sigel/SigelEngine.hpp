#pragma once

#include <GLFW/glfw3.h>
#include "vkapi/VulkanContext.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "Utils.hpp"
#include "Scene.hpp"

namespace sigel
{
    constexpr uint32_t WIDTH = 1920;
    constexpr uint32_t HEIGHT = 1080;
    
    class SigelEngine {
        public:
            GLFWwindow *window;
            VulkanContext vctx;
            PipelineManager pipelineManager;
            ResourceManager resourceManager;
            Renderer renderer;
            DefaultScene defaultScene;
            IScene* activeScene = nullptr;
            MovementInput input;
        public:
            SigelEngine() = default;
            void run();
            void loadScene(IScene* scene);
        private:
            void initWindow();
            void initEngine();
            void mainLoop();
            void waitIdle();
            void cleanup();

            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    };
}