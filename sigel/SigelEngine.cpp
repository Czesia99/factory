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
        vctx.init(window);
        status("CORE", "Vulkan context ready");
        
        resourceManager.init(&vctx.allocator);
        shaderManager.init(&vctx.device);
        status("SHADER MANAGER", "Initialized");

        auto shaderCode = readFile("../sigel/shaders/slang2.spv");
        auto shaderModule = shaderManager.createShaderModule(shaderCode);
        status("SHADER MANAGER", "Slang SPIR-V binary loaded");

        pipeline.init(&vctx.swapchain, &vctx.device);
        pipeline.createDescriptorSetLayout();
        pipeline.createGraphicsPipeline(shaderModule);
        status("PIPELINE", "Graphics Pipeline created");
        

        renderer.init(&vctx.device, &vctx.swapchain, &pipeline, &resourceManager);
        status("RENDERER", "Initialization..");
        
        renderer.createCommandPool();
        status("RENDERER", "Command Pool allocated");

        // renderer.loadObject(cube_vertices, cube_indices);

        if (renderer.loadedObjects.size() > 0)
        {
            renderer.createDescriptorPool();
            status("RENDERER", "Descriptor Pool allocated");
            renderer.createDescriptorSets();
            status("RENDERER", "Descriptor Sets allocated");
        }
        renderer.createFrameData();
        status("RENDERER", "Frame Data created");
    }

    void SigelEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            renderer.drawFrame();
        }
        vctx.waitIdle();
    }

    void SigelEngine::cleanup()
    {
        renderer.cleanupObjects();
        resourceManager.cleanup();
        vctx.clean();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->renderer.framebufferResized = true;
    }
}