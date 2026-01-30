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

        window = glfwCreateWindow(WIDTH, HEIGHT, "FACTORY", nullptr, nullptr);
    }

    void SigelEngine::initVulkan()
    {
        instance.init();
        printf("instance created\n");
        surface.createSurface(instance.getInstance(), window);
        printf("surface created\n");
        physicalDevice.pickPhysicalDevice(instance.getInstance());
        printf("physical device selected\n");
        physicalDevice.printDeviceInfo();
        logicalDevice.createLogicalDevice(physicalDevice.getDevice(), surface.getSurface());
        printf("logical device created\n");
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