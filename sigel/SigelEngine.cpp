#include "SigelEngine.hpp"
#include <iostream>
#include <chrono>

namespace sigel
{
    void SigelEngine::run()
    {
        try {
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
        glfwSetKeyCallback(window, keyCallbackWrapper);
        glfwSetCursorPosCallback(window, mouseCallbackWrapper);
    }

    void SigelEngine::initEngine()
    {
        vctx.init(window);
        status("CORE", "Vulkan context ready");
        
        addScene("default", new DefaultScene());
    }

    void SigelEngine::mainLoop()
    {
        auto last = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(window)) {
            inputManager.update();
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            if (!pendingScene && !activeScene)
            {
                status("ENGINE", "no scene provided, loading default scene");
                drawScene("default");
            }

            if (pendingScene)
            {
                loadScene(pendingScene);
                pendingScene = nullptr;
            }

            if (activeScene) {
                activeScene->onUpdate(dt);
            }

            vctx.renderer.drawFrame(*activeScene);
        }
        vctx.waitIdle();
    }

    void SigelEngine::cleanup()
    {
        vctx.renderer.cleanupRenderObjects();
        vctx.resourceManager.cleanup();
        vctx.clean();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::addScene(const std::string& name, IScene* scene)
    {
        scenes[name] = scene;
    }

    void SigelEngine::drawScene(const std::string& name)
    {
        status("SCENE MANAGER", "Drawing [" + name + "] Scene");
        pendingScene = scenes.at(name);
    }

    void SigelEngine::loadScene(IScene* scene)
    {
        vctx.waitIdle();
        if (activeScene)
        {
            activeScene->onExit(vctx.resourceManager, vctx.pipelineManager);
            vctx.renderer.cleanupRenderObjects();
        }
        scene->onEnter(vctx.resourceManager, vctx.pipelineManager);
        vctx.renderer.prepareScene(*scene);
        activeScene = scene;
    }

    void SigelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->vctx.renderer.framebufferResized = true;
    }

    void SigelEngine::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onKey(key, action);
    }

    void SigelEngine::mouseCallbackWrapper(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onMouseMove(static_cast<float>(x), static_cast<float>(y));
    }
}