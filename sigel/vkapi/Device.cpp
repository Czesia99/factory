#include "Device.hpp"
#include "../Utils.hpp"

namespace sigel
{
    void Device::pickPhysicalDevice(vk::raii::Instance &instance)
    {
        std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
        const auto devIter = std::ranges::find_if(devices,
        [&](auto const & device) {
                auto queueFamilies = device.getQueueFamilyProperties();
                bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
                const auto qfpIter = std::ranges::find_if(queueFamilies,
                []( vk::QueueFamilyProperties const & qfp )
                        {
                            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
                        } );
                isSuitable = isSuitable && ( qfpIter != queueFamilies.end() );
                auto extensions = device.enumerateDeviceExtensionProperties( );
                bool found = true;
                for (auto const & extension : deviceExtensions) {
                    auto extensionIter = std::ranges::find_if(extensions, [extension](auto const & ext) {return strcmp(ext.extensionName, extension) == 0;});
                    found = found &&  extensionIter != extensions.end();
                }
                isSuitable = isSuitable && found;
                if (isSuitable) {
                    physicalDevice = device;
                }
                return isSuitable;
        });
        if (devIter == devices.end()) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
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