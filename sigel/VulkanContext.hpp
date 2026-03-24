#pragma once

#include "Instance.hpp"
#include "WindowSurface.hpp"
#include "Device.hpp"

namespace sigel
{
    class VulkanContext
    {
        public:
            Instance     instance;
            WindowSurface surface;
            Device       device;

            void init(GLFWwindow* window)
            {
                instance.init();
                surface.createSurface(instance.getInstance(), window);
                device.pickPhysicalDevice(instance.getInstance());
                device.printDeviceInfo();
                device.createLogicalDevice(surface.getSurface());
            }

            void waitIdle() { device.logicalDevice.waitIdle(); }
    };
}