#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace sigel
{
    #ifdef NDEBUG
        constexpr bool enableValidationLayers = false;
    #else
        constexpr bool enableValidationLayers = true;
    #endif

    const std::vector<char const*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
        // "VK_LAYER_KHRONOS_profiles",
        // "VK_LAYER_KHRONOS_timeline_semaphore",
        // "VK_LAYER_KHRONOS_shader_object",
        // "VK_LAYER_KHRONOS_synchronization2"
        //"VK_LAYER_MESA_screenshot",
        //"VK_LAYER_MESA_vram_report_limit",
        //"VK_LAYER_MESA_overlay"
    };

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }
}