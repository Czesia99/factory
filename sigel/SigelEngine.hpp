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
            
            MovementInput input;
            double mouse_x;
            double mouse_y;
        private:
            std::unordered_map<std::string, IScene*> scenes;
            IScene *activeScene = nullptr;
            IScene *sceneToLoad = nullptr;

        public:
            static SigelEngine& get() {
                static SigelEngine instance;
                return instance;
            }

            SigelEngine(const SigelEngine&) = delete;
            SigelEngine& operator=(const SigelEngine&) = delete;

            void run();
            void addScene(const std::string& name, IScene* scene);
            void queueScene(const std::string& name);
            
        private:
            SigelEngine() { initWindow(); initEngine(); }
            void loadScene(IScene* scene);
            void initWindow();
            void initEngine();
            void mainLoop();
            void waitIdle();
            void cleanup();

            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            static void mouseCallbackWrapper(GLFWwindow* window, double x, double y);

    };
}