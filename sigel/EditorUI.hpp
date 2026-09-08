#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "vkapi/VulkanContext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <GLFW/glfw3.h>

namespace sigel
{
    class EditorUI
    {
        public:
            void init(GLFWwindow *window, VulkanContext &vctx);
            void update(Scene *scene);
            void swapMode();
            void cleanup();
        private:
            void cameraSettingsFrame(Scene *scene);
            void lightSettingsFrame(Scene *scene);
        public:
            bool display = false;
        private:
            VkDescriptorPool imguiPool = VK_NULL_HANDLE;
            VkPipelineCache  pipelineCache = VK_NULL_HANDLE;

            GLFWwindow* _window = nullptr;
            VkDevice _logicalDevice = VK_NULL_HANDLE;
    };
}
