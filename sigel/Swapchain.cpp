#include "Swapchain.hpp"

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "Utils.hpp"

namespace sigel
{
    void Swapchain::init(VulkanContext *vctx, GLFWwindow *window)
    {
        _vctx = vctx;
        _window = window;
    }

    void Swapchain::createSwapChain()
    {
        auto surfaceCapabilities = _vctx->device.physicalDevice.getSurfaceCapabilitiesKHR(*_vctx->surface.getSurface());
        swapChainSurfaceFormat = chooseSwapSurfaceFormat(_vctx->device.physicalDevice.getSurfaceFormatsKHR(*_vctx->surface.getSurface()));
        swapChainExtent = chooseSwapExtent(surfaceCapabilities);
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        minImageCount = ( surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount ) ? surfaceCapabilities.maxImageCount : minImageCount;

        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = _vctx->surface.getSurface(),
            .minImageCount = minImageCount,
            .imageFormat = swapChainSurfaceFormat.format,
            .imageColorSpace = swapChainSurfaceFormat.colorSpace,
            .imageExtent = swapChainExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = chooseSwapPresentMode(_vctx->device.physicalDevice.getSurfacePresentModesKHR(*_vctx->surface.getSurface())), 
            .clipped = true,
            .oldSwapchain = nullptr
        };

        uint32_t gIndex = _vctx->device.graphicsIndex;
        uint32_t pIndex = _vctx->device.presentIndex;
        uint32_t queueFamilyIndices[] = { gIndex, pIndex };

        if (gIndex != pIndex) {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
            status("SWAPCHAIN", "queue family concurrent mode");
        } else {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
            swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
            status("SWAPCHAIN", "queue family exclusif mode");
        }

        swapChain = vk::raii::SwapchainKHR(_vctx->device.logicalDevice, swapChainCreateInfo );
        swapChainImages = swapChain.getImages();
    }

    void Swapchain::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_window, &width, &height);
            glfwWaitEvents();
        }
        _vctx->device.logicalDevice.waitIdle();
        
        cleanupSwapChain();

        createSwapChain();
        createImageViews();
    }

    void Swapchain::createImageViews()
    {
        swapChainImageViews.clear();
        vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
		for (auto& image : swapChainImages)
		{
			imageViewCreateInfo.image = image;
			swapChainImageViews.emplace_back(_vctx->device.logicalDevice, imageViewCreateInfo);
		}
    }

    void Swapchain::cleanupSwapChain()
    {
        swapChainImageViews.clear();
        swapChain = nullptr;
    }

    vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    
    vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }
}