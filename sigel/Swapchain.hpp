#pragma once

#define NOMINMAX
#include <vulkan/vulkan_raii.hpp>

#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "WindowSurface.hpp"

namespace sigel
{
    class Swapchain 
    {
        public:
            vk::raii::SwapchainKHR swapChain = nullptr;
            std::vector<vk::Image> swapChainImages;
            vk::SurfaceFormatKHR swapChainSurfaceFormat;
            vk::Extent2D swapChainExtent;
            std::vector<vk::raii::ImageView> swapChainImageViews;
        private:
            PhysicalDevice& _pDevice;
            LogicalDevice&  _lDevice;
            WindowSurface&  _surface;
            GLFWwindow* _window;

        public:
            Swapchain(PhysicalDevice& pDevice, LogicalDevice& lDevice, WindowSurface& surface, GLFWwindow* window);
            void createSwapChain();
        private:
            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
            vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
            vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    };
}