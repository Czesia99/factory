#include "SigelEngine.hpp"
#include <iostream>
#include <chrono>

namespace sigel
{
    void SigelEngine::run()
    {
        initWindow();
        try {
            initEngine();
            mainLoop();
        } catch (const std::exception& e) {
            std::cerr << "Fatal: " << e.what() << "\n";
        }
        cleanup();
    }

    void SigelEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "FACTORY", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void SigelEngine::initEngine()
    {
        vctx.init(window);
        status("CORE", "Vulkan context ready");
        
        resourceManager.init(&vctx);
        pipelineManager.init(&vctx.swapchain, &vctx.device);
        renderer.init(&vctx, &resourceManager, &pipelineManager);
        status("RENDERER", "Initialization..");
        
        loadScene(&defaultScene);
    }

    void SigelEngine::mainLoop()
    {
        auto last = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            input.moveForward = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            input.moveBackward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
            input.moveLeft = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            input.moveRight = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            input.moveUp = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            input.moveDown = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);

            if (activeScene) {
                activeScene->onUpdate(dt);
                activeScene->processInput(input, dt);
            }
            renderer.drawFrame(*activeScene);
        }
        vctx.waitIdle();
    }

    void SigelEngine::cleanup()
    {
        renderer.cleanupRenderObjects();
        resourceManager.cleanup();
        vctx.clean();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::loadScene(IScene* scene)
    {
        vctx.waitIdle();
        if (activeScene) activeScene->onExit(resourceManager, pipelineManager);
        scene->onEnter(resourceManager, pipelineManager);
        renderer.prepareScene(*scene);
        activeScene = scene;
    }

    void SigelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->renderer.framebufferResized = true;
    }
}