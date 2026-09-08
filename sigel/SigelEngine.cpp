#include "SigelEngine.hpp"
#include <iostream>
#include <chrono>

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

#include <tiny_obj_loader.h>

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
        editor.init(window, vctx);
        status("CORE", "Vulkan context ready");
        vctx.resourceManager.createTextureImage("../assets/textures/texture0.jpg");
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

            if (!nextActiveScene && !activeScene)
            {
                status("ENGINE", "no scene provided, loading default scene");
                drawScene("default");
            }

            if (nextActiveScene)
            {
                loadScene(nextActiveScene);
                nextActiveScene = nullptr;
            }

            if (activeScene)
            {
                activeScene->onUpdate(dt);
                editor.update(activeScene);
            }

            vctx.renderer.drawFrame(*activeScene, editor.display);
        }
        vctx.waitIdle();
    }

    void SigelEngine::cleanup()
    {
        editor.cleanup();

        vctx.renderer.cleanupRenderObjects();
        vctx.resourceManager.cleanup();

        for (auto& [name, scene] : scenes)
        {
            if (scene)
            {
                scene->onDestroy();
                delete scene;
            }
        }

        // scenes.clear();
        vctx.clean();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::addScene(const std::string& name, Scene* scene)
    {
        scenes[name] = scene;
    }

    void SigelEngine::drawScene(const std::string& name)
    {
        if (scenes.contains(name))
        {
            status("SCENE MANAGER", "Drawing [" + name + "] Scene");
            nextActiveScene = scenes.at(name);
        }
        else
        {
            status("SCENE MANAGER", "Scene [" + name + "] does not exist");
        }
    }

    void SigelEngine::loadScene(Scene* scene)
    {
        vctx.waitIdle();

        if (activeScene)
        {
            activeScene->onExit();
            vctx.renderer.cleanupRenderObjects();
        }

        if (!scene->isSetup)
        {
            scene->onSetup();
            scene->isSetup = true;
        }

        scene->onEnter();
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
