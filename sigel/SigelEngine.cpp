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
        status("CORE", "Vulkan Instance initialized");

        surface.createSurface(instance.getInstance(), window);
        status("SURFACE", "GLFW Window Surface created");

        physicalDevice.pickPhysicalDevice(instance.getInstance());
        status("GPU", "Hardware selection complete");

        physicalDevice.printDeviceInfo();
        logicalDevice.createLogicalDevice(physicalDevice.getDevice(), surface.getSurface());
        status("DEVICE", "Logical Device and Queues ready");

        swapchain.init(&physicalDevice, &logicalDevice, &surface, window);
        swapchain.createSwapChain();
        swapchain.createImageViews();
        status("SWAPCHAIN", "Swapchain images allocated");

        shaderManager.init(&logicalDevice);
        status("SHADER MANAGER", "Initialized");

        auto shaderCode = readFile("../sigel/shaders/slang.spv");
        auto shaderModule = shaderManager.createShaderModule(shaderCode);
        status("SHADER MANAGER", "Slang SPIR-V binary loaded");

        pipeline.init(&swapchain, &logicalDevice);
        pipeline.createGraphicsPipeline(shaderModule);
        status("PIPELINE", "Graphics Pipeline created");

        renderer.init(&logicalDevice, &swapchain);
        status("RENDERER", "Initialization..");
        renderer.createCommandPool();
        status("RENDERER", "Command Pool allocated");
        renderer.createCommandBuffer();
        status("RENDERER", "Command Buffer allocated");


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