#include "Renderer.hpp"
#include "Vertex.hpp"
#include "frames.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

namespace sigel
{
    void Renderer::init(Device *device, Swapchain *swapchain, Pipeline *pipeline)
    {
        _device = device;
        _swapchain = swapchain;
        _pipeline = pipeline;
    }

    void Renderer::loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        GameObject object;
        //TODO: check if mesh is already loaded in cache
        std::cout << "create object" << std::endl;
        object.mesh.vertexBuffer = createVertexBuffer2(vertices, commandPool, _device);
        std::cout << "create vertex buffer" << std::endl;
        createIndexBuffer2(object.mesh.indexBuffer, indices, commandPool, _device);
        std::cout << "create index buffer" << std::endl;
        createUniformBuffers2(object.uniformBuffers, _device);
        std::cout << "create unifrom buffers" << std::endl;
        loadedObjects.emplace_back(std::move(object));
    }



    void Renderer::drawFrame()
    {
		auto fenceResult = _device->logicalDevice.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);        
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
        
		auto [result, imageIndex] = _swapchain->swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            _swapchain->recreateSwapChain();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        _device->logicalDevice.resetFences(*inFlightFences[frameIndex]);
        auto &cmd = commandBuffers[frameIndex];
        // commandBuffers[frameIndex].reset();
        cmd.reset();

        updateUniformBuffer(frameIndex);
        recordCommandBuffer(imageIndex);

        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
		const vk::SubmitInfo submitInfo{.waitSemaphoreCount   = 1,
                                        .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
                                        .pWaitDstStageMask    = &waitDestinationStageMask,
                                        .commandBufferCount   = 1,
                                        .pCommandBuffers      = &*commandBuffers[frameIndex],
                                        .signalSemaphoreCount = 1,
                                        .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]};
        
        _device->graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

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
        
        result = _device->presentQueue.presentKHR(presentInfoKHR);

        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR))
        {
            framebufferResized = false;
            _swapchain->recreateSwapChain();
        }
        else
        {
            // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
            assert(result == vk::Result::eSuccess);
        }


        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::createSyncObjects()
    {
        assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

		for (size_t i = 0; i < _swapchain->swapChainImages.size(); i++)
		{
			renderFinishedSemaphores.emplace_back(_device->logicalDevice, vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			presentCompleteSemaphores.emplace_back(_device->logicalDevice, vk::SemaphoreCreateInfo());
			inFlightFences.emplace_back(_device->logicalDevice, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
		}
    }

    void Renderer::createCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = _device->graphicsIndex};
        commandPool = vk::raii::CommandPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::createDescriptorPool()
    {
        vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
        vk::DescriptorPoolCreateInfo poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = MAX_FRAMES_IN_FLIGHT, .poolSizeCount = 1, .pPoolSizes = &poolSize };
        descriptorPool = vk::raii::DescriptorPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::createDescriptorSets() {
        for (auto& obj : loadedObjects) 
        {
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *_pipeline->descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool = *descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            obj.descriptorSets = _device->logicalDevice.allocateDescriptorSets(allocInfo);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DescriptorBufferInfo bufferInfo{
                    .buffer = *obj.uniformBuffers[i].buffer, 
                    .offset = 0, 
                    .range = sizeof(UniformBufferObject)
                };

                vk::WriteDescriptorSet descriptorWrite{
                    .dstSet = *obj.descriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &bufferInfo
                };

                _device->logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);
            }
        }
    }
    // void Renderer::createCommandBuffer()
    // {
    //     vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    //     commandBuffer = std::move(vk::raii::CommandBuffers(_device->logicalDevice, allocInfo).front());
    // }

    void Renderer::createCommandBuffers()
    {
        commandBuffers.clear();
        vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
        commandBuffers = vk::raii::CommandBuffers(_device->logicalDevice, allocInfo);
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 
            static_cast<float>(_swapchain->swapChainExtent.width) / _swapchain->swapChainExtent.height, 0.1f, 10.0f);
        proj[1][1] *= -1;

        for (size_t i = 0; i < loadedObjects.size(); i++)
        {
            UniformBufferObject ubo{};
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, 0.0f));
            ubo.model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = view;
            ubo.proj = proj;
            memcpy(loadedObjects[i].uniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));
        }
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
        // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->graphicsPipeline);
        commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, *_pipeline->graphicsPipeline);

        for (auto &obj : loadedObjects)
        {
            commandBuffers[frameIndex].bindVertexBuffers(0, *obj.mesh.vertexBuffer.buffer, {0});
            commandBuffers[frameIndex].bindIndexBuffer(*obj.mesh.indexBuffer.buffer, 0, vk::IndexType::eUint32 );  
            commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline->pipelineLayout, 0, *obj.descriptorSets[frameIndex], nullptr);
        }
        
        commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapchain->swapChainExtent.width), static_cast<float>(_swapchain->swapChainExtent.height), 0.0f, 1.0f));
        commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchain->swapChainExtent));
        // commandBuffer.draw(3, 1, 0, 0);
        // commandBuffers[frameIndex].draw(3, 1, 0, 0);
        commandBuffers[frameIndex].drawIndexed(cube_indices.size(), 1, 0, 0, 0);
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