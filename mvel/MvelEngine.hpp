#pragma once

#include <GLFW/glfw3.h>
#include "vkapi/VulkanContext.hpp"
#include "Utils.hpp"
#include "Scene.hpp"
#include "EditorUI.hpp"
#include <mvel/ecs/Coordinator.hpp>
#include <mvel/ecs/systems/RenderSystem.hpp>

namespace mvel
{
    constexpr uint32_t WIDTH = 1920;
    constexpr uint32_t HEIGHT = 1080;

    class MvelEngine {
        public:
            GLFWwindow *window;
            VulkanContext vctx;
            InputManager inputManager;
            EditorUI editor;
            Coordinator coordinator;
            std::shared_ptr<RenderSystem> renderSystem;


        private:
            std::unordered_map<std::string, Scene*> scenes;
            Scene *activeScene = nullptr;
            Scene *nextActiveScene = nullptr;

        public:
            static MvelEngine& get() {
                static MvelEngine instance;
                return instance;
            }

            MvelEngine(const MvelEngine&) = delete;
            MvelEngine& operator=(const MvelEngine&) = delete;

            void run();
            void addScene(const std::string& name, Scene* scene);
            void drawScene(const std::string& name);

        private:
            MvelEngine() { initWindow(); initEngine(); }
            void initWindow();
            void initEngine();
            void mainLoop();
            void waitIdle();
            void cleanup();

            void loadScene(Scene* scene);

            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouseCallbackWrapper(GLFWwindow* window, double x, double y);

    };
}
