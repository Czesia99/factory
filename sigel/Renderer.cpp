#include "Renderer.hpp"

namespace sigel
{
    void Renderer::createCommandPool(LogicalDevice &lDevice)
    {
        vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = lDevice.graphicsIndex};
        commandPool = vk::raii::CommandPool(lDevice.getDevice(), poolInfo);
    }

    void Renderer::createCommandBuffer(LogicalDevice &lDevice)
    {
        vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
        commandBuffer = std::move(vk::raii::CommandBuffers(lDevice.getDevice(), allocInfo).front());
    }

    // void Renderer::recordCommandBuffer(uint32_t imageIndex)
    // {
    //     commandBuffer.begin({});

    //     // Transition the image layout for rendering
    //     transition_image_layout(
    //         imageIndex,
    //         vk::ImageLayout::eUndefined,
    //         vk::ImageLayout::eColorAttachmentOptimal,
    //         {},
    //         vk::AccessFlagBits2::eColorAttachmentWrite,
    //         vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    //         vk::PipelineStageFlagBits2::eColorAttachmentOutput
    //     );

    //     // Set up the color attachment
    //     vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    //     vk::RenderingAttachmentInfo attachmentInfo = {
    //         .imageView = swapChainImageViews[imageIndex],
    //         .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    //         .loadOp = vk::AttachmentLoadOp::eClear,
    //         .storeOp = vk::AttachmentStoreOp::eStore,
    //         .clearValue = clearColor
    //     };

    //     // Set up the rendering info
    //     vk::RenderingInfo renderingInfo = {
    //         .renderArea = { .offset = { 0, 0 }, .extent = swapChainExtent },
    //         .layerCount = 1,
    //         .colorAttachmentCount = 1,
    //         .pColorAttachments = &attachmentInfo
    //     };

    //     // Begin rendering
    //     commandBuffer.beginRendering(renderingInfo);

    //     // Rendering commands will go here

    //     // End rendering
    //     commandBuffer.endRendering();

    //     // Transition the image layout for presentation
    //     transition_image_layout(
    //         imageIndex,
    //         vk::ImageLayout::eColorAttachmentOptimal,
    //         vk::ImageLayout::ePresentSrcKHR,
    //         vk::AccessFlagBits2::eColorAttachmentWrite,
    //         {},
    //         vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    //         vk::PipelineStageFlagBits2::eBottomOfPipe
    //     );

    //     commandBuffer.end();
    // }
}