#include "Swapchain.hpp"

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "Utils.hpp"

namespace sigel
{
    void Swapchain::init(PhysicalDevice *pDevice, LogicalDevice *lDevice, WindowSurface *surface, GLFWwindow *window)
    {
        _pDevice = pDevice;
        _lDevice = lDevice;
        _surface = surface;
        _window = window;
    }

    void Swapchain::createSwapChain()
    {
        auto surfaceCapabilities = _pDevice->getDevice().getSurfaceCapabilitiesKHR(*_surface->getSurface());
        swapChainSurfaceFormat = chooseSwapSurfaceFormat(_pDevice->getDevice().getSurfaceFormatsKHR(*_surface->getSurface()));
        swapChainExtent = chooseSwapExtent(surfaceCapabilities);
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        minImageCount = ( surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount ) ? surfaceCapabilities.maxImageCount : minImageCount;

        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = *_surface->getSurface(),
            .minImageCount = minImageCount,
            .imageFormat = swapChainSurfaceFormat.format,
            .imageColorSpace = swapChainSurfaceFormat.colorSpace,
            .imageExtent = swapChainExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = chooseSwapPresentMode(_pDevice->getDevice().getSurfacePresentModesKHR(*_surface->getSurface())), 
            .clipped = true,
            .oldSwapchain = nullptr
        };

        uint32_t gIndex = _lDevice->graphicsIndex;
        uint32_t pIndex = _lDevice->presentIndex;
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

        swapChain = vk::raii::SwapchainKHR( _lDevice->getDevice(), swapChainCreateInfo );
        swapChainImages = swapChain.getImages();
    }

    void Swapchain::createImageViews()
    {
        swapChainImageViews.clear();
        vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
		for (auto& image : swapChainImages)
		{
			imageViewCreateInfo.image = image;
			swapChainImageViews.emplace_back(_lDevice->getDevice(), imageViewCreateInfo);
		}
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