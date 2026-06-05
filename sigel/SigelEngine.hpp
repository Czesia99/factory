#pragma once

#include <GLFW/glfw3.h>
#include "vkapi/VulkanContext.hpp"
#include "Utils.hpp"
#include "Scene.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace sigel
{
    constexpr uint32_t WIDTH = 1920;
    constexpr uint32_t HEIGHT = 1080;
    
    class SigelEngine {
        public:
            GLFWwindow *window;
            VulkanContext vctx;
            InputManager inputManager;

            VkDescriptorPool imguiPool = VK_NULL_HANDLE;
            VkPipelineCache  pipelineCache = VK_NULL_HANDLE;

            double mouse_x;
            double mouse_y;

            bool showEditor = false;
        private:
            std::unordered_map<std::string, IScene*> scenes;
            IScene *activeScene = nullptr;
            IScene *nextActiveScene = nullptr;

        public:
            static SigelEngine& get() {
                static SigelEngine instance;
                return instance;
            }

            SigelEngine(const SigelEngine&) = delete;
            SigelEngine& operator=(const SigelEngine&) = delete;

            void run();
            void addScene(const std::string& name, IScene* scene);
            void drawScene(const std::string& name);

            void editorModeSwap();

        private:
            SigelEngine() { initWindow(); initEngine(); }
            void initWindow();
            void initEngine();
            void mainLoop();
            void waitIdle();
            void cleanup();
            
            void loadScene(IScene* scene);

            void initImgui();

            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouseCallbackWrapper(GLFWwindow* window, double x, double y);

    };
}