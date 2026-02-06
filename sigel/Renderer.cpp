#include "Renderer.hpp"
#include <iostream>

namespace sigel
{
    void Renderer::init(LogicalDevice *lDevice, Swapchain *swapchain, Pipeline *pipeline)
    {
        _lDevice = lDevice;
        _swapchain = swapchain;
        _pipeline = pipeline;
    }

    void Renderer::drawFrame()
    {
		auto fenceResult = _lDevice->getDevice().waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);        
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
		_lDevice->getDevice().resetFences(*inFlightFences[frameIndex]);
        
		auto [result, imageIndex] = _swapchain->swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
        
        auto &cmd = commandBuffers[frameIndex];
        // commandBuffers[frameIndex].reset();
        cmd.reset();

        recordCommandBuffer(imageIndex);

        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
		const vk::SubmitInfo submitInfo{.waitSemaphoreCount   = 1,
                                        .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
                                        .pWaitDstStageMask    = &waitDestinationStageMask,
                                        .commandBufferCount   = 1,
                                        .pCommandBuffers      = &*commandBuffers[frameIndex],
                                        .signalSemaphoreCount = 1,
                                        .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]};
        
        _lDevice->graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

        // vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, {},
        // vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
        // {}, vk::AccessFlagBits::eColorAttachmentWrite);
        
        // renderPassInfo.dependencyCount = 1;
        // renderPassInfo.pDependencies = &dependency;
        
		const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
		                                        .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
		                                        .swapchainCount     = 1,
		                                        .pSwapchains        = &*_swapchain->swapChain,
		                                        .pImageIndices      = &imageIndex};
        
        result = _lDevice->presentQueue.presentKHR(presentInfoKHR);
		switch (result)
		{
			case vk::Result::eSuccess:
				break;
			case vk::Result::eSuboptimalKHR:
				std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
				break;
			default:
				break;        // an unexpected result is returned!
		}
        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::createSyncObjects()
    {
        assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

		for (size_t i = 0; i < _swapchain->swapChainImages.size(); i++)
		{
			renderFinishedSemaphores.emplace_back(_lDevice->getDevice(), vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			presentCompleteSemaphores.emplace_back(_lDevice->getDevice(), vk::SemaphoreCreateInfo());
			inFlightFences.emplace_back(_lDevice->getDevice(), vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
		}
    }

    void Renderer::createCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = _lDevice->graphicsIndex};
        commandPool = vk::raii::CommandPool(_lDevice->getDevice(), poolInfo);
    }

    // void Renderer::createCommandBuffer()
    // {
    //     vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    //     commandBuffer = std::move(vk::raii::CommandBuffers(_lDevice->getDevice(), allocInfo).front());
    // }

    void Renderer::createCommandBuffers()
    {
        commandBuffers.clear();
        vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
        commandBuffers = vk::raii::CommandBuffers(_lDevice->getDevice(), allocInfo);
    }

    void Renderer::recordCommandBuffer(uint32_t imageIndex)
    {
        auto &commandBuffer = commandBuffers[frameIndex];
        commandBuffer.begin({});
        transition_image_layout(
            imageIndex,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo = {
            .imageView = _swapchain->swapChainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clearColor
        };

        vk::RenderingInfo renderingInfo = {
            .renderArea = { .offset = { 0, 0 }, .extent = _swapchain->swapChainExtent },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        commandBuffer.beginRendering(renderingInfo);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->graphicsPipeline);
        commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapchain->swapChainExtent.width), static_cast<float>(_swapchain->swapChainExtent.height), 0.0f, 1.0f));
        commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchain->swapChainExtent));

        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRendering();

        transition_image_layout(
            imageIndex,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
            {},                                                     // dstAccessMask
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
            vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
        );

        commandBuffer.end();
    }

    void Renderer::transition_image_layout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    ) {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = srcStageMask,
            .srcAccessMask = srcAccessMask,
            .dstStageMask = dstStageMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = _swapchain->swapChainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
    }
}