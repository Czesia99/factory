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
        drawScene("default");
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
            input.changeScene = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS);

            if (pendingScene)
            {
                loadScene(pendingScene);
                pendingScene = nullptr;
            }

            if (activeScene) {
                activeScene->onUpdate(dt);
                activeScene->processInput(input, dt);
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

        if (app->activeScene != nullptr) {
            app->activeScene->keyCallback(key, scancode, action, mods);
        }
    }

    void SigelEngine::mouseCallbackWrapper(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        static bool firstMouse = true;

        double dx = 0.0;
        double dy = 0.0;

        if (firstMouse) {
            app->mouse_x = x;
            app->mouse_y = y;
            firstMouse = false;
        } else {
            dx = x - app->mouse_x;
            dy = y - app->mouse_y;

            app->mouse_x = x;
            app->mouse_y = y;
        }

        if (app->activeScene) {
            app->activeScene->mouseCallback(static_cast<float>(dx), static_cast<float>(dy));
        }
    }

}