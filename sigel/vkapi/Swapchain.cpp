#include "Swapchain.hpp"

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "../Utils.hpp"

namespace sigel
{
    void Swapchain::init(Device *device, WindowSurface *surface, GLFWwindow *window, GpuAllocator *allocator)
    {
        _device = device;
        _surface = surface;
        _window = window;
        _allocator = allocator;
    }

    void Swapchain::createSwapChain()
    {
        auto surfaceCapabilities = _device->physicalDevice.getSurfaceCapabilitiesKHR(*_surface->getSurface());
        swapChainSurfaceFormat = chooseSwapSurfaceFormat(_device->physicalDevice.getSurfaceFormatsKHR(*_surface->getSurface()));
        swapChainExtent = chooseSwapExtent(surfaceCapabilities);
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        minImageCount = ( surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount ) ? surfaceCapabilities.maxImageCount : minImageCount;

        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = _surface->getSurface(),
            .minImageCount = minImageCount,
            .imageFormat = swapChainSurfaceFormat.format,
            .imageColorSpace = swapChainSurfaceFormat.colorSpace,
            .imageExtent = swapChainExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = chooseSwapPresentMode(_device->physicalDevice.getSurfacePresentModesKHR(*_surface->getSurface())), 
            .clipped = true,
            .oldSwapchain = nullptr
        };

        uint32_t gIndex = _device->graphicsIndex;
        uint32_t pIndex = _device->presentIndex;
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

        swapChain = vk::raii::SwapchainKHR(_device->logicalDevice, swapChainCreateInfo );
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
        _device->logicalDevice.waitIdle();
        
        cleanupDepthResources();
        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createDepthResources();
    }

    void Swapchain::createImageViews()
    {
        swapChainImageViews.clear();
        vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
		for (auto& image : swapChainImages)
		{
			imageViewCreateInfo.image = image;
			swapChainImageViews.emplace_back(_device->logicalDevice, imageViewCreateInfo);
		}
    }

    void Swapchain::createDepthResources()
    {
        depthImage = _allocator->createImage(
            swapChainExtent.width,
            swapChainExtent.height,
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        vk::ImageViewCreateInfo viewInfo{
            .image    = depthImage.image,
            .viewType = vk::ImageViewType::e2D,
            .format   = depthFormat,
            .subresourceRange = {
                .aspectMask     = vk::ImageAspectFlagBits::eDepth,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };
        depthImageView = vk::raii::ImageView(_device->logicalDevice, viewInfo);
    }

    void Swapchain::cleanupDepthResources()
    {
        depthImageView.clear();
        _allocator->destroyImage(depthImage);
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