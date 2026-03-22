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

        device.pickPhysicalDevice(instance.getInstance());
        status("GPU", "Hardware selection complete");

        device.printDeviceInfo();
        device.createLogicalDevice(surface.getSurface());
        status("DEVICE", "Logical Device and Queues ready");

        swapchain.init(&device, &surface, window);
        swapchain.createSwapChain();
        swapchain.createImageViews();
        status("SWAPCHAIN", "Swapchain images allocated");
        
        resourceManager.init(&device);

        shaderManager.init(&device);
        status("SHADER MANAGER", "Initialized");

        auto shaderCode = readFile("../sigel/shaders/slang2.spv");
        auto shaderModule = shaderManager.createShaderModule(shaderCode);
        status("SHADER MANAGER", "Slang SPIR-V binary loaded");

        pipeline.init(&swapchain, &device);
        pipeline.createDescriptorSetLayout();
        pipeline.createGraphicsPipeline(shaderModule);
        status("PIPELINE", "Graphics Pipeline created");
        

        renderer.init(&device, &swapchain, &pipeline, &resourceManager);
        status("RENDERER", "Initialization..");
        
        renderer.createCommandPool();
        status("RENDERER", "Command Pool allocated");

        renderer.loadObject(cube_vertices, cube_indices);

        renderer.createDescriptorPool();
        status("RENDERER", "Descriptor Pool allocated");
        
        renderer.createDescriptorSets();
        status("RENDERER", "Descriptor Sets allocated");

        renderer.createFrameData();
        status("RENDERER", "Frame Data created");
    }

    void SigelEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            renderer.drawFrame();
        }
        waitIdle();
    }

    void SigelEngine::waitIdle()
    {
        device.logicalDevice.waitIdle();
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