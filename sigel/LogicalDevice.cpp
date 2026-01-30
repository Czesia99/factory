#include "LogicalDevice.hpp"

namespace sigel
{
    void LogicalDevice::createLogicalDevice(vk::raii::PhysicalDevice &pdevice, vk::raii::SurfaceKHR &surface)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = pdevice.getQueueFamilyProperties();

		auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const &qfp) { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); });
		assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() && "No graphics queue family found!");

		auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

        auto presentIndex = pdevice.getSurfaceSupportKHR(graphicsIndex, *surface)
                                            ? graphicsIndex
                                            : static_cast<uint32_t>(queueFamilyProperties.size());
        
        if (presentIndex == queueFamilyProperties.size()) {
            // the graphicsIndex doesn't support present -> look for another family index that supports both
            // graphics and present
            for (size_t i = 0; i < queueFamilyProperties.size(); i++)
            {
                if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                    pdevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
                {
                    graphicsIndex = static_cast<uint32_t>( i );
                    presentIndex  = graphicsIndex;
                    break;
                }
            }
            if ( presentIndex == queueFamilyProperties.size() )
            {
                // there's nothing like a single family index that supports both graphics and present -> look for another
                // family index that supports present
                for (size_t i = 0; i < queueFamilyProperties.size(); i++)
                {
                    if ( pdevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
                    {
                        presentIndex = static_cast<uint32_t>(i);
                        break;
                    }
                }
            }
        }
        if ((graphicsIndex == queueFamilyProperties.size()) || (presentIndex == queueFamilyProperties.size()))
        {
            throw std::runtime_error( "Could not find a queue for graphics or present -> terminating" );
        }

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		    {},                                   // vk::PhysicalDeviceFeatures2
		    {.dynamicRendering = true},           // vk::PhysicalDeviceVulkan13Features
		    {.extendedDynamicState = true}        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		};

        std::vector deviceExtensions = { vk::KHRSwapchainExtensionName };

        float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = graphicsIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
		vk::DeviceCreateInfo deviceCreateInfo { .pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                                .queueCreateInfoCount    = 1,
                                                .pQueueCreateInfos       = &deviceQueueCreateInfo,
                                                .enabledExtensionCount   = deviceExtensions.size(),
                                                .ppEnabledExtensionNames = deviceExtensions.data()};

		device = vk::raii::Device(pdevice, deviceCreateInfo);
		graphicsQueue = vk::raii::Queue(device, graphicsIndex, 0);
        presentQueue = vk::raii::Queue( device, presentIndex, 0 );
    }
}