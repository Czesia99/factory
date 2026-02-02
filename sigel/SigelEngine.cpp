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
        swapchain.init(&physicalDevice, &logicalDevice, &surface, window);
        printf("swapchain initialisation\n");
        swapchain.createSwapChain();
        printf("swapchain created\n");
        swapchain.createImageViews();
        printf("image views created\n");

        shaderManager.init(&logicalDevice);
        auto shaderCode = readFile("../sigel/shaders/slang.spv");
        printf("readfile on shader\n");

        pipeline.init(&shaderManager);



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