#include "SigelEngine.hpp"
#include <iostream>
namespace sigel
{
    void SigelEngine::run()
    {
        initWindow();
        try {
            initVulkan();
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
    }

    void SigelEngine::initVulkan()
    {
        vctx.init(window);
        status("CORE", "Vulkan context ready");
        
        resourceManager.init(&vctx);
        pipelineManager.init(&vctx.swapchain, &vctx.device);
        renderer.init(&vctx, &resourceManager, &pipelineManager);
        status("RENDERER", "Initialization..");
        
        uint32_t texID = resourceManager.createTextureImage("../flo.jpg");
        renderer.loadObject(cube_vertices, cube_indices, PipelineType::DEFAULT, texID);

        status("CACA", "texture loaded");

        if (renderer.loadedObjects.size() > 0)
        {
            renderer.createDescriptorPool();
            status("RENDERER", "Descriptor Pool allocated");
            renderer.createDescriptorSets();
            status("RENDERER", "Descriptor Sets allocated");
        }
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