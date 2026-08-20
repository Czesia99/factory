#include "Device.hpp"
#include "../Utils.hpp"

namespace sigel
{
    void Device::pickPhysicalDevice(vk::raii::Instance &instance)
    {
        std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
        
        int bestScore = -1;
        
        for (const auto& device : devices) {
            auto props = device.getProperties();
            auto queueFamilies = device.getQueueFamilyProperties();

            bool supportsApi = props.apiVersion >= VK_API_VERSION_1_3;
            
            bool hasGraphicsQueue = std::ranges::any_of(queueFamilies, [](const auto& qfp) {
                return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
            });

            auto extensions = device.enumerateDeviceExtensionProperties();
            bool hasExtensions = true;
            for (const char* extension : deviceExtensions) {
                bool found = std::ranges::any_of(extensions, [extension](const auto& ext) {
                    return strcmp(ext.extensionName, extension) == 0;
                });
                hasExtensions = hasExtensions && found;
            }

            bool isSuitable = supportsApi && hasGraphicsQueue && hasExtensions;

            if (!isSuitable) continue;

            int score = 0;
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                score += 1000;
            } else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
                score += 10;
            }

            if (score > bestScore) {
                bestScore = score;
                physicalDevice = device;
            }
        }

        if (bestScore == -1) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        maxMsaaSamples = getMaxUsableSampleCount();
        msaaSamples = getBalancedSampleCount();
    }

    void Device::createLogicalDevice(vk::raii::SurfaceKHR &surface)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const &qfp) { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); });
		assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() && "No graphics queue family found!");

		graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

        presentIndex = physicalDevice.getSurfaceSupportKHR(graphicsIndex, *surface)
                                            ? graphicsIndex
                                            : static_cast<uint32_t>(queueFamilyProperties.size());

        if (presentIndex == queueFamilyProperties.size()) {
            // the graphicsIndex doesn't support present -> look for another family index that supports both
            // graphics and present
            for (size_t i = 0; i < queueFamilyProperties.size(); i++)
            {
                if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                    physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
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
                    if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface))
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

        vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
            vk::PhysicalDeviceVulkan14Features
        > featureChain = {
            { .features = {.samplerAnisotropy = true } },
            { .shaderDrawParameters = vk::True },
            { .synchronization2 = true, .dynamicRendering = true },
            { .extendedDynamicState = true },
            { .maintenance5 = true }
        };

        float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = graphicsIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
		vk::DeviceCreateInfo deviceCreateInfo { .pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                                .queueCreateInfoCount    = 1,
                                                .pQueueCreateInfos       = &deviceQueueCreateInfo,
                                                .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
                                                .ppEnabledExtensionNames = deviceExtensions.data()};

		logicalDevice = vk::raii::Device(physicalDevice, deviceCreateInfo);
		graphicsQueue = vk::raii::Queue(logicalDevice, graphicsIndex, 0);
        presentQueue = vk::raii::Queue(logicalDevice, presentIndex, 0);
    }

    uint32_t Device::findQueueFamilies(vk::raii::PhysicalDevice physicalDevice)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        auto graphicsQueueFamilyProperty =
        std::find_if( queueFamilyProperties.begin(),
                        queueFamilyProperties.end(),
                        []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

        return static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );
    }

    vk::SampleCountFlagBits Device::getMaxUsableSampleCount()
    {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

        vk::SampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        if (counts & vk::SampleCountFlagBits::e64)  return vk::SampleCountFlagBits::e64;
        if (counts & vk::SampleCountFlagBits::e32)  return vk::SampleCountFlagBits::e32;
        if (counts & vk::SampleCountFlagBits::e16)  return vk::SampleCountFlagBits::e16;
        if (counts & vk::SampleCountFlagBits::e8)   return vk::SampleCountFlagBits::e8;
        if (counts & vk::SampleCountFlagBits::e4)   return vk::SampleCountFlagBits::e4;
        if (counts & vk::SampleCountFlagBits::e2)   return vk::SampleCountFlagBits::e2;

        return vk::SampleCountFlagBits::e1;
    }

    vk::SampleCountFlagBits Device::getBalancedSampleCount()
    {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        vk::SampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

        if (counts & vk::SampleCountFlagBits::e4) return vk::SampleCountFlagBits::e4;
        if (counts & vk::SampleCountFlagBits::e2) return vk::SampleCountFlagBits::e2;
        if (counts & vk::SampleCountFlagBits::e8) return vk::SampleCountFlagBits::e8;

        return vk::SampleCountFlagBits::e1;
    }

    void Device::printDeviceInfo()
    {
        vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
        std::string gpuName = "Selected GPU: " + std::string(props.deviceName.data());
        std::string gpuType = "Type: " + vk::to_string(props.deviceType);
        status("GPU", gpuName);
        status("GPU", gpuType);
        printf("Vulkan API Version: %d.%d.%d\n",
            VK_API_VERSION_MAJOR(props.apiVersion),
            VK_API_VERSION_MINOR(props.apiVersion),
            VK_API_VERSION_PATCH(props.apiVersion));
    }
}
