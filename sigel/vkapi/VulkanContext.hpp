#pragma once

#include "Instance.hpp"
#include "WindowSurface.hpp"
#include "Device.hpp"
#include "GpuAllocator.hpp"
#include "Swapchain.hpp"

namespace sigel
{
    class VulkanContext
    {
        public:
            Instance     instance;
            WindowSurface surface;
            Device       device;
            GpuAllocator allocator;
            Swapchain swapchain;

            void init(GLFWwindow* window)
            {
                instance.init();
                surface.createSurface(instance.getInstance(), window);
                device.pickPhysicalDevice(instance.getInstance());
                device.printDeviceInfo();
                device.createLogicalDevice(surface.getSurface());
                allocator.init(&device, &instance);
                swapchain.init(&device, &surface, window, &allocator);
                swapchain.createSwapChain();
                swapchain.createImageViews();
                swapchain.createDepthResources();
            }

            void waitIdle() { device.logicalDevice.waitIdle(); }

            void clean()
            {
                swapchain.cleanupDepthResources();
                swapchain.cleanupImageViews();
                allocator.cleanup();
            }
    };
}