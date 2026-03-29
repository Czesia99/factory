#include "Pipeline.hpp"
#include "Vertex.hpp"
#include "Utils.hpp"
namespace sigel
{
    void PipelineManager::init(Swapchain *swapchain, Device *device)
    {
        _swapchain = swapchain;
        _device = device;

        createPipeline();
    }

    const PipelineInstance& PipelineManager::getPipeline(uint32_t id) const
    {
        return pipelines[id];
    }

    uint32_t PipelineManager::createPipeline(PipelineConfig config)
    {
        PipelineInstance instance;
        instance.name = config.name;

        instance.descriptorSetLayout = createDescriptorSetLayout();

        auto code = readFile(config.shaderPath);

        vk::ShaderModuleCreateInfo createInfo{
            .sType    = vk::StructureType::eShaderModuleCreateInfo,
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(code.data())
        };

        vk::PipelineShaderStageCreateInfo vertexStage{.pNext = &createInfo, .stage = vk::ShaderStageFlagBits::eVertex, .module = VK_NULL_HANDLE,  .pName = "vertMain"};
        vk::PipelineShaderStageCreateInfo fragmentStage{.pNext = &createInfo, .stage = vk::ShaderStageFlagBits::eFragment, .module = VK_NULL_HANDLE, .pName = "fragMain"};
        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

        //pipeline vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInput  {   .vertexBindingDescriptionCount   = 1,
                                                                .pVertexBindingDescriptions      = &bindingDescription,
                                                                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                                                                .pVertexAttributeDescriptions    = attributeDescriptions.data() };
        
        
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly {.topology = vk::PrimitiveTopology::eTriangleList};
        
        vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };
        
        //hey
        vk::Viewport{ 0.0f, 0.0f, static_cast<float>(_swapchain->swapChainExtent.width), static_cast<float>(_swapchain->swapChainExtent.height), 0.0f, 1.0f };
        vk::Rect2D{vk::Offset2D{ 0, 0 }, _swapchain->swapChainExtent};


        vk::PipelineRasterizationStateCreateInfo rasterizer {
            .depthClampEnable = vk::False, 
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = config.polygonMode,
            .cullMode = config.cullMode,
            .frontFace = config.frontFace,
            .depthBiasEnable = vk::False,
            .depthBiasSlopeFactor = 1.0f, 
            .lineWidth = 1.0f 
        };

        vk::PipelineMultisampleStateCreateInfo multisampling {.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

        vk::PipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable    = vk::False,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp =  vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments =  &colorBlendAttachment };


        vk::PipelineDepthStencilStateCreateInfo depthStencil{
            .depthTestEnable       = config.depthTest  ? vk::True : vk::False,
            .depthWriteEnable      = config.depthWrite ? vk::True : vk::False,
            .depthCompareOp        = vk::CompareOp::eLess,
            .depthBoundsTestEnable = vk::False,
            .stencilTestEnable     = vk::False
        };

        std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};
        
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &*instance.descriptorSetLayout, .pushConstantRangeCount = 0};
        instance.pipelineLayout = vk::raii::PipelineLayout(_device->logicalDevice, pipelineLayoutInfo);
        
        vk::PipelineRenderingCreateInfo renderingInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &(_swapchain->swapChainSurfaceFormat.format), .depthAttachmentFormat = config.depthFormat };

        vk::GraphicsPipelineCreateInfo pipelineInfo{
            .pNext               = &renderingInfo,
            .stageCount          = 2,
            .pStages             = shaderStages,
            .pVertexInputState   = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState      = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState   = &multisampling,
            .pDepthStencilState  = &depthStencil,
            .pColorBlendState    = &colorBlending,
            .pDynamicState       = &dynamicState,
            .layout              = *instance.pipelineLayout,
            .renderPass          = nullptr
        };

        instance.pipeline = vk::raii::Pipeline(_device->logicalDevice, nullptr, pipelineInfo);
        uint32_t id = static_cast<uint32_t>(pipelines.size());
        pipelines.emplace_back(std::move(instance));
        return id;
    }

    vk::raii::DescriptorSetLayout PipelineManager::createDescriptorSetLayout()
    {
        vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);
        vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1, .pBindings = &uboLayoutBinding};
        return vk::raii::DescriptorSetLayout(_device->logicalDevice, layoutInfo);
    }
}