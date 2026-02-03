#include "PhysicalDevice.hpp"
#include "Utils.hpp"

namespace sigel
{
    void PhysicalDevice::pickPhysicalDevice(vk::raii::Instance &instance)
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

    uint32_t PhysicalDevice::findQueueFamilies(vk::raii::PhysicalDevice physicalDevice) 
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        auto graphicsQueueFamilyProperty =
        std::find_if( queueFamilyProperties.begin(),
                        queueFamilyProperties.end(),
                        []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

        return static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );
    }

    void PhysicalDevice::printDeviceInfo()
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