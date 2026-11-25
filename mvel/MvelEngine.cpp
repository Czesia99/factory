#include "MvelEngine.hpp"
#include <iostream>
#include <chrono>

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

namespace mvel
{
    void MvelEngine::run()
    {
        try {
            mainLoop();
        } catch (const std::exception& e) {
            std::cerr << "Fatal: " << e.what() << "\n";
        }
        cleanup();
    }

    void MvelEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "FACTORY", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallbackWrapper);
        glfwSetCursorPosCallback(window, mouseCallbackWrapper);
    }

    void MvelEngine::initEngine()
    {
        vctx.init(window);
        editor.init(window, vctx);

        //ECS Setup
        coordinator.Init();

        coordinator.RegisterComponent<Transform>();
        coordinator.RegisterComponent<RenderComponent>();

        renderSystem = coordinator.RegisterSystem<RenderSystem>();

        Signature signature;
        signature.set(coordinator.GetComponentType<Transform>());
	    signature.set(coordinator.GetComponentType<RenderComponent>());

        coordinator.SetSystemSignature<RenderSystem>(signature);

        vctx.resourceManager.createTextureImage("../assets/textures/texture0.jpg");
        addScene("default", new DefaultScene());
    }

    void MvelEngine::mainLoop()
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
                renderSystem->Update(coordinator);
                editor.update(activeScene);

                const auto& cam = activeScene->getCamera();
                CameraData camData{
                    .view = cam.getViewMatrix(),
                    .proj = cam.getProjectionMatrix(vctx.renderer.aspectRatio()),
                    .pos  = cam.settings.pos
                };

                vctx.renderer.drawFrame(renderSystem->items(), camData, activeScene->getLight(), editor.display);
            }

        }
        vctx.waitIdle();
    }

    void MvelEngine::cleanup()
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

    void MvelEngine::addScene(const std::string& name, Scene* scene)
    {
        scenes[name] = scene;
    }

    void MvelEngine::drawScene(const std::string& name)
    {
        if (scenes.contains(name))
        {
            status("SCENE MANAGER", "Drawing Scene [" + name + "]");
            nextActiveScene = scenes.at(name);
        }
        else
        {
            status("SCENE MANAGER", "Scene [" + name + "] does not exist");
        }
    }

    void MvelEngine::loadScene(Scene* scene)
    {
        vctx.waitIdle();

        if (activeScene)
        {
            activeScene->onExit();
            activeScene->destroyEntities();
            vctx.renderer.cleanupRenderObjects();
        }

        if (!scene->isSetup)
        {
            scene->onSetup();
            scene->isSetup = true;
        }

        scene->onEnter();
        activeScene = scene;
    }

    void MvelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<MvelEngine*>(glfwGetWindowUserPointer(window));
        app->vctx.renderer.framebufferResized = true;
    }

    void MvelEngine::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<MvelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onKey(key, action);
    }

    void MvelEngine::mouseCallbackWrapper(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<MvelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onMouseMove(static_cast<float>(x), static_cast<float>(y));
    }
}
