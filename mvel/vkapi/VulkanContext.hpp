#pragma once

#include "Instance.hpp"
#include "WindowSurface.hpp"
#include "Device.hpp"
#include "GpuAllocator.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "ResourceManager.hpp"
#include "Renderer.hpp"
#include <mvel/Utils.hpp>

namespace mvel
{
    class VulkanContext
    {
        public:
            Instance     instance;
            WindowSurface surface;
            Device       device;
            GpuAllocator allocator;
            Swapchain swapchain;
            PipelineManager pipelineManager;
            ResourceManager resourceManager;
            Renderer renderer;

            void init(GLFWwindow* window)
            {
                instance.init();
                surface.createSurface(instance.getInstance(), window);
                device.pickPhysicalDevice(instance.getInstance());
                device.createLogicalDevice(surface.getSurface());
                allocator.init(&device, &instance);
                swapchain.init(&device, &surface, window, &allocator);
                swapchain.createSwapChain();
                swapchain.createImageViews();
                swapchain.createColorResources();
                swapchain.createDepthResources();
                pipelineManager.init(&swapchain, &device);
                resourceManager.init(&allocator, &device);
                renderer.init(&device, &swapchain, &pipelineManager, &resourceManager);
                status("CORE", "Vulkan context ready");
            }

            void waitIdle() { device.logicalDevice.waitIdle(); }

            void clean()
            {
                swapchain.cleanup();
                allocator.cleanup();
            }
    };
}
