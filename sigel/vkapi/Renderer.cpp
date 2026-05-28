#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace sigel
{
    void Renderer::init(Device *device, Swapchain *swapchain, PipelineManager *pipelineManager, ResourceManager *resourceManager)
    {
        _device = device;
        _swapchain = swapchain;
        _pipelineManager = pipelineManager;
        _resourceManager = resourceManager;

        createCommandPool();
        createFrameData();
    }

    void Renderer::loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t pipelineID, uint32_t textureID)
    {
        RenderObject object;
        object.meshID = _resourceManager->loadMesh(vertices, indices);
        object.pipelineID = pipelineID;
        object.textureID = textureID;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            object.uniformBuffers.emplace_back(_resourceManager->createUniformBuffer(sizeof(UniformBufferObject)));
        }
        renderObjects.emplace_back(std::move(object));
    }

    void Renderer::prepareScene(const IScene& scene)
    {
        cleanupRenderObjects();
        descriptorPool.clear();

        const auto& sceneObjects = scene.getObjects();
        if (sceneObjects.empty()) return;

        for (const auto& so : sceneObjects) {
            RenderObject ro;
            ro.pipelineID = so.pipelineID;
            ro.meshID = so.meshID;
            ro.textureID = so.textureID;
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                ro.uniformBuffers.emplace_back(_resourceManager->createUniformBuffer(sizeof(UniformBufferObject)));
            renderObjects.emplace_back(std::move(ro));
        }

        createDescriptorPool();
        createDescriptorSets();
    }

    void Renderer::cleanupRenderObjects()
    {
        for (auto& obj : renderObjects)
        {
            for (auto& ubo : obj.uniformBuffers)
                _resourceManager->destroyBuffer(ubo);
            obj.descriptorSets.clear();
        }
        renderObjects.clear();
    }
    
    void Renderer::drawFrame(const IScene& scene)
    {
        auto &frame = currentFrame();

        waitFence();

		auto [result, imageIndex] = _swapchain->swapChain.acquireNextImage(UINT64_MAX, *frame.presentSemaphore, nullptr);

        checkImageResult(result);
        
        _device->logicalDevice.resetFences(*frame.inFlightFence);
        
        updateUniformBuffer(frameIndex, scene);
        frame.commandBuffer.reset();
        recordCommandBuffer(imageIndex);

        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
		const vk::SubmitInfo submitInfo{.waitSemaphoreCount   = 1,
                                        .pWaitSemaphores      = &*frame.presentSemaphore,
                                        .pWaitDstStageMask    = &waitDestinationStageMask,
                                        .commandBufferCount   = 1,
                                        .pCommandBuffers      = &*frame.commandBuffer,
                                        .signalSemaphoreCount = 1,
                                        .pSignalSemaphores    = &*renderSemaphores[imageIndex]};
        
        _device->graphicsQueue.submit(submitInfo, *frame.inFlightFence);

        
		const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
		                                        .pWaitSemaphores    = &*renderSemaphores[imageIndex],
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

    void Renderer::createFrameData()
    {
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool        = *commandPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };
        auto commandBuffers = vk::raii::CommandBuffers(_device->logicalDevice, allocInfo);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            frames[i].commandBuffer   = std::move(commandBuffers[i]);
            frames[i].inFlightFence   = vk::raii::Fence(_device->logicalDevice, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
            frames[i].presentSemaphore = vk::raii::Semaphore(_device->logicalDevice, vk::SemaphoreCreateInfo{});
        }

        for (size_t i = 0; i < _swapchain->swapChainImages.size(); i++)
            renderSemaphores.emplace_back(_device->logicalDevice, vk::SemaphoreCreateInfo{});
    }

    void Renderer::createCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = _device->graphicsIndex};
        commandPool = vk::raii::CommandPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::createDescriptorPool()
    {
        uint32_t maxSets = static_cast<uint32_t>(renderObjects.size()) * MAX_FRAMES_IN_FLIGHT;
        std::array poolSize {
            vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, maxSets),
            vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, maxSets)
        };

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets       = maxSets,
            .poolSizeCount = poolSize.size(),
            .pPoolSizes    = poolSize.data()
        };

        descriptorPool = vk::raii::DescriptorPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::createDescriptorSets() {
        for (auto &obj : renderObjects) 
        {
            const PipelineInstance &pipeline = _pipelineManager->getPipeline(obj.pipelineID);
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *pipeline.descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool = *descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            obj.descriptorSets = _device->logicalDevice.allocateDescriptorSets(allocInfo);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DescriptorBufferInfo bufferInfo{
                    .buffer = vk::Buffer(obj.uniformBuffers[i].buffer),
                    .offset = 0, 
                    .range = sizeof(UniformBufferObject)
                };
                vk::DescriptorImageInfo  imageInfo{
                    .sampler = _resourceManager->textures[obj.textureID].sampler, 
                    .imageView = _resourceManager->textures[obj.textureID].view, 
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                };

                std::array<vk::WriteDescriptorSet, 2> descriptorWrites{{{
                    .dstSet          = *obj.descriptorSets[i],
                    .dstBinding      = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType  = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo     = &bufferInfo
                },
                {
                    .dstSet          = *obj.descriptorSets[i],
                    .dstBinding      = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo      = &imageInfo
                }}};

                _device->logicalDevice.updateDescriptorSets(descriptorWrites, nullptr);
            }
        }

        
    }

    FrameData& Renderer::currentFrame()
    {
        return frames[frameIndex];
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage, const IScene& scene) 
    {
        const auto& sceneObjects = scene.getObjects();
        const auto& sceneCamera = scene.getCamera();

        float width  = static_cast<float>(_swapchain->swapChainExtent.width);
        float height = static_cast<float>(_swapchain->swapChainExtent.height);
        float aspect = width / height;

        for (size_t i = 0; i < renderObjects.size(); i++) {
            UniformBufferObject ubo{};
            ubo.model = sceneObjects[i].transform.getModelMatrix();
            ubo.view  = sceneCamera.getViewMatrix();
            ubo.proj  = sceneCamera.getProjectionMatrix(aspect);
            memcpy(renderObjects[i].uniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));
        }
    }

    void Renderer::recordCommandBuffer(uint32_t imageIndex)
    {
        auto &cmd = currentFrame().commandBuffer;
        cmd.begin({});
        transition_image_layout(
            cmd,
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

        vk::ImageMemoryBarrier2 depthBarrier{
            .srcStageMask        = vk::PipelineStageFlagBits2::eEarlyFragmentTests,
            .srcAccessMask       = {},
            .dstStageMask        = vk::PipelineStageFlagBits2::eEarlyFragmentTests,
            .dstAccessMask       = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .oldLayout           = vk::ImageLayout::eUndefined,
            .newLayout           = vk::ImageLayout::eDepthAttachmentOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = _swapchain->depthImage.image,
            .subresourceRange    = {
                .aspectMask  = vk::ImageAspectFlagBits::eDepth,
                .levelCount  = 1,
                .layerCount  = 1
            }
        };
        vk::DependencyInfo depthDep{
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers    = &depthBarrier
        };
        cmd.pipelineBarrier2(depthDep);

        vk::RenderingAttachmentInfo depthAttachment{
            .imageView   = *_swapchain->depthImageView,
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eDontCare,
            .clearValue  = vk::ClearDepthStencilValue{ 1.0f, 0 }
        };

        vk::RenderingInfo renderingInfo = {
            .renderArea = { .offset = { 0, 0 }, .extent = _swapchain->swapChainExtent },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo,
            .pDepthAttachment = &depthAttachment
        };

        cmd.beginRendering(renderingInfo);
        
        cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapchain->swapChainExtent.width), static_cast<float>(_swapchain->swapChainExtent.height), 0.0f, 1.0f));
        cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchain->swapChainExtent));
        
        for (int i = 0; i < renderObjects.size(); i++)
        {
            const Mesh &mesh = _resourceManager->getMesh(renderObjects[i].meshID);
            const PipelineInstance &pipeline = _pipelineManager->getPipeline(renderObjects[i].pipelineID);
            vk::Buffer vb = mesh.vertexBuffer.buffer;
            vk::Buffer ib = mesh.indexBuffer.buffer;
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);
            cmd.bindVertexBuffers(0, vb, {0});
            cmd.bindIndexBuffer(ib, 0, vk::IndexType::eUint32 );  
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline.pipelineLayout, 0, *renderObjects[i].descriptorSets[frameIndex], nullptr);
            cmd.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
        }
        
        cmd.endRendering();

        transition_image_layout(
            cmd,
            imageIndex,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
            {},                                                     // dstAccessMask
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
            vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
        );

        cmd.end();
    }

    void Renderer::waitFence()
    {
        auto &fence = currentFrame().inFlightFence;
		auto fenceResult = _device->logicalDevice.waitForFences(*fence, vk::True, UINT64_MAX);        
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
    }

    void Renderer::checkImageResult(vk::Result result)
    {
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
    }

    void Renderer::transition_image_layout(
        vk::raii::CommandBuffer &cmd,
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

        cmd.pipelineBarrier2(dependencyInfo);
    }
}