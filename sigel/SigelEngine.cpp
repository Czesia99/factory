#include "SigelEngine.hpp"

namespace sigel
{
    void SigelEngine::run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    void SigelEngine::initWindow()
    {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void SigelEngine::initVulkan()
    {
        instance.createInstance();
        printf("instance created\n");
    }

    void SigelEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void SigelEngine::cleanup()
    {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
}