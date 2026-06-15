#include "EditorUI.hpp"
#include "SigelEngine.hpp"

namespace sigel
{
    void EditorUI::init(GLFWwindow *window, VulkanContext &vctx)
    {
        _window = window;
        _logicalDevice = *vctx.device.logicalDevice;

        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE },
            { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        vkCreateDescriptorPool(*vctx.device.logicalDevice, &pool_info, nullptr, &imguiPool);
        // check_vk_result(err);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.ApiVersion = VK_API_VERSION_1_4;
        init_info.Instance = *vctx.instance.instance;
        init_info.PhysicalDevice = *vctx.device.physicalDevice;
        init_info.Device = *vctx.device.logicalDevice;
        init_info.QueueFamily = vctx.device.graphicsIndex;
        init_info.Queue = *vctx.device.graphicsQueue;
        init_info.PipelineCache = pipelineCache;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = vctx.swapchain.swapChainImages.size();
        init_info.Allocator = nullptr;
        init_info.UseDynamicRendering = true;

        static VkFormat colorFormat = static_cast<VkFormat>(vctx.swapchain.swapChainSurfaceFormat.format);
        static VkFormat depthFormat = static_cast<VkFormat>(vctx.swapchain.depthFormat);    
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&init_info);
    }

    void EditorUI::update()
    {
        if (!display) return;
        
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); 
        ImGui::Render();
    }

    void EditorUI::swapMode()
    {
        if (!display) {
            display = true;
            glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            display = false;
            glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    void EditorUI::cleanup()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(_logicalDevice, imguiPool, nullptr);
    }
}