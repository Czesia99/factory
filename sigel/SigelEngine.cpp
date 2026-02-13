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

        window = glfwCreateWindow(WIDTH, HEIGHT, "FACTORY", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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

        vertexManager.init(&logicalDevice, &physicalDevice);
        
        pipeline.init(&swapchain, &logicalDevice);
        pipeline.createGraphicsPipeline(shaderModule);
        status("PIPELINE", "Graphics Pipeline created");
        
        renderer.init(&logicalDevice, &swapchain, &pipeline, &vertexManager);
        status("RENDERER", "Initialization..");
        renderer.createCommandPool();
        status("RENDERER", "Command Pool allocated");
        renderer.createCommandBuffers();
        status("RENDERER", "Command Buffer allocated");
        renderer.createSyncObjects();
        status("RENDERER", "Sync Objects created");

        vertexManager.createVertexBuffer(triangle_vertices, renderer.commandPool);


    }

    void SigelEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            renderer.drawFrame();
        }

        logicalDevice.getDevice().waitIdle();
    }

    void SigelEngine::cleanup()
    {
        swapchain.cleanupSwapChain();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->renderer.framebufferResized = true;
    }
}