#pragma once

#define NOMINMAX
#include <vulkan/vulkan_raii.hpp>

#include "VulkanContext.hpp"
#include "GpuAllocator.hpp"

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

            AllocatedImage      depthImage     = {};
            vk::raii::ImageView depthImageView = nullptr;
            vk::Format          depthFormat    = vk::Format::eD32Sfloat;

        private:
            VulkanContext *_vctx = nullptr;
            GLFWwindow *_window = nullptr;
            GpuAllocator *_allocator = nullptr;

        public:
            Swapchain() = default;
            void init(VulkanContext *vctx, GLFWwindow *window, GpuAllocator *allocator);
            void createSwapChain();
            void recreateSwapChain();
            void createImageViews();
            void createDepthResources();
            void cleanupDepthResources();
            void cleanupSwapChain();
        private:
            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
            vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
            vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    };
}