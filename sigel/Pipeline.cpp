#include "Pipeline.hpp"
#include "Utils.hpp"

#include "Vertex.hpp"

namespace sigel
{
    void Pipeline::init(Swapchain *swapchain, Device *device)
    {
        _swapchain = swapchain;
        _device = device;
    }

    void Pipeline::createGraphicsPipeline(vk::raii::ShaderModule &shaderModule)
    {
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule,  .pName = "vertMain"};
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        //pipeline vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{ .vertexBindingDescriptionCount   = 1,
                                                                .pVertexBindingDescriptions      = &bindingDescription,
                                                                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                                                                .pVertexAttributeDescriptions    = attributeDescriptions.data() };
        
        //
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly {.topology = vk::PrimitiveTopology::eTriangleList};
        
        vk::Viewport{ 0.0f, 0.0f, static_cast<float>(_swapchain->swapChainExtent.width), static_cast<float>(_swapchain->swapChainExtent.height), 0.0f, 1.0f };
        vk::Rect2D{vk::Offset2D{ 0, 0 }, _swapchain->swapChainExtent};

        // vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1);
        vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };

        vk::PipelineRasterizationStateCreateInfo rasterizer {
            .depthClampEnable = vk::False, 
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasSlopeFactor = 1.0f, 
            .lineWidth = 1.0f 
        };

        vk::PipelineMultisampleStateCreateInfo multisampling {.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

        vk::PipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable    = vk::False,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        // color blend
        // colorBlendAttachment.blendEnable = vk::True;
        // colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        // colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        // colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        // colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        // colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        // colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp =  vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments =  &colorBlendAttachment };

        std::vector dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &*descriptorSetLayout, .pushConstantRangeCount = 0};

        pipelineLayout = vk::raii::PipelineLayout(_device->logicalDevice, pipelineLayoutInfo);

        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &(_swapchain->swapChainSurfaceFormat.format) };

        vk::GraphicsPipelineCreateInfo pipelineInfo { 
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 2, .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState, .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling, .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState, .layout = pipelineLayout, .renderPass = nullptr 
        };

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        graphicsPipeline = vk::raii::Pipeline(_device->logicalDevice, nullptr, pipelineInfo);
    }

    void Pipeline::createDescriptorSetLayout()
    {
        vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);
        vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1, .pBindings = &uboLayoutBinding};
        descriptorSetLayout = vk::raii::DescriptorSetLayout(_device->logicalDevice, layoutInfo);
    }
}