#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

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

    void Renderer::cleanupRenderObjects()
    {
        materialSets.clear();
        globalDescriptorSets.clear();

        for (auto& ubo : uniformBuffers) { _resourceManager->destroyBuffer(ubo); }
        uniformBuffers.clear();

        for (auto& bo : objectSSBOs) { _resourceManager->destroyBuffer(bo);}
        objectSSBOs.clear();

        objectCapacity = 0;
    }

    void Renderer::drawFrame(std::span<const RenderItem> items, const CameraData& camera, const DirLight& light, bool showEditor)
    {
        auto &frame = currentFrame();

        if (_pipelineManager->msaaChanged)
        {
            _device->logicalDevice.waitIdle();
            _swapchain->recreateSwapChain();
            _pipelineManager->recreateAllPipelines();
            _pipelineManager->msaaChanged = false;
        }

        ensureResources(items.size());
        waitFence();

		auto [result, imageIndex] = _swapchain->swapChain.acquireNextImage(UINT64_MAX, *frame.presentSemaphore, nullptr);

        checkImageResult(result);

        _device->logicalDevice.resetFences(*frame.inFlightFence);

        updateUniformBuffer(frameIndex, items, camera, light);
        frame.commandBuffer.reset();
        recordCommandBuffer(imageIndex, items, showEditor);

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
        std::array poolSize {
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, MAX_FRAMES_IN_FLIGHT),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_MATERIAL_SETS * 4),
        };

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets       = MAX_FRAMES_IN_FLIGHT + MAX_MATERIAL_SETS,
            .poolSizeCount = poolSize.size(),
            .pPoolSizes    = poolSize.data()
        };

        descriptorPool = vk::raii::DescriptorPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::ensureResources(size_t objectCount)
    {
        if (objectCount == 0) return;

        const bool firstInit = uniformBuffers.empty();
        if (!firstInit && objectCount <= objectCapacity) return;

        _device->logicalDevice.waitIdle();

        if (firstInit)
        {
            createDescriptorPool();

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                uniformBuffers.emplace_back(_resourceManager->createUniformBuffer(sizeof(UniformBufferObject)));

            const PipelineInstance& defaultPipeline = _pipelineManager->getPipeline(0);
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *defaultPipeline.globalDescriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool     = *descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts        = layouts.data()
            };
            globalDescriptorSets = _device->logicalDevice.allocateDescriptorSets(allocInfo);
        }

        for (auto& bo : objectSSBOs) _resourceManager->destroyBuffer(bo);
        objectSSBOs.clear();

        objectCapacity = std::max({ objectCount, objectCapacity * 2, size_t{ 64 } });
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            objectSSBOs.emplace_back(_resourceManager->createStorageBuffer(objectCapacity * sizeof(ObjBufferObject)));

        writeGlobalDescriptorSets();
    }

    void Renderer::writeGlobalDescriptorSets()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorBufferInfo uboInfo{ .buffer = vk::Buffer(uniformBuffers[i].buffer), .offset = 0, .range = sizeof(UniformBufferObject) };
            vk::DescriptorBufferInfo ssboInfo{ .buffer = vk::Buffer(objectSSBOs[i].buffer), .offset = 0, .range = VK_WHOLE_SIZE };

            std::array<vk::WriteDescriptorSet, 2> writes{{
                { .dstSet = *globalDescriptorSets[i], .dstBinding = 0, .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &uboInfo },
                { .dstSet = *globalDescriptorSets[i], .dstBinding = 1, .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer, .pBufferInfo = &ssboInfo }
            }};
            _device->logicalDevice.updateDescriptorSets(writes, nullptr);
        }
    }

    vk::DescriptorSet Renderer::getMaterialSet(uint32_t pipelineID, const Material& m)
    {
        MaterialKey key{ pipelineID, m.diffuseID, m.metallicID, m.roughnessID, m.normalID };
        if (auto it = materialSets.find(key); it != materialSets.end())
            return *it->second;

        const PipelineInstance& pipeline = _pipelineManager->getPipeline(pipelineID);
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool     = *descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts        = &*pipeline.materialDescriptorSetLayout
        };
        auto sets = _device->logicalDevice.allocateDescriptorSets(allocInfo);

        const std::array<uint32_t, 4> texIDs{ m.diffuseID, m.metallicID, m.roughnessID, m.normalID };
        std::array<vk::DescriptorImageInfo, 4> infos;
        std::array<vk::WriteDescriptorSet, 4> writes;

        for (uint32_t i = 0; i < 4; i++)
        {
            const auto& tex = _resourceManager->textures[texIDs[i]];
            infos[i] = {
                .sampler = tex.sampler,
                .imageView = tex.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            };

            writes[i] = {
                .dstSet = *sets[0],
                .dstBinding = i,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler, .pImageInfo = &infos[i]
            };
        }
        _device->logicalDevice.updateDescriptorSets(writes, nullptr);

        return *materialSets.emplace(key, std::move(sets[0])).first->second;
    }

    FrameData& Renderer::currentFrame()
    {
        return frames[frameIndex];
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage, std::span<const RenderItem> items, const CameraData& camera, const DirLight& light)
    {
        if (uniformBuffers.empty()) return;

        UniformBufferObject ubo{};
        ubo.view   = camera.view;
        ubo.proj   = camera.proj;
        ubo.light  = light;
        ubo.camPos = camera.pos;
        memcpy(uniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));

        auto* dst = static_cast<ObjBufferObject*>(objectSSBOs[currentImage].mapped);
        for (size_t i = 0; i < items.size(); i++)
            dst[i].model = items[i].model;
    }

    void Renderer::recordCommandBuffer(uint32_t imageIndex, std::span<const RenderItem> items, bool showEditor)
    {
        auto &cmd = currentFrame().commandBuffer;
        cmd.begin({});

        transition_image_layout(
            cmd,
            _swapchain->swapChainImages[imageIndex],
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        if (_device->msaaSamples != vk::SampleCountFlagBits::e1)
        {
            transition_image_layout(
                cmd,
                _swapchain->msaaImage.image,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                {},
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput
            );
        }

        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo colorAttachment = {
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .clearValue = clearColor
        };

        if (_device->msaaSamples != vk::SampleCountFlagBits::e1)
        {
            colorAttachment.imageView = _swapchain->msaaImage.view;
            colorAttachment.resolveMode = vk::ResolveModeFlagBits::eAverage;
            colorAttachment.resolveImageView = *_swapchain->swapChainImageViews[imageIndex];
            colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        }
        else
        {
            colorAttachment.imageView = *_swapchain->swapChainImageViews[imageIndex];
            colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
            colorAttachment.resolveImageView = VK_NULL_HANDLE;
            colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        }

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
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthAttachment
        };

        cmd.beginRendering(renderingInfo);

        cmd.setViewport(
            0,
            vk::Viewport(
                0.0f,
                static_cast<float>(_swapchain->swapChainExtent.height),
                static_cast<float>(_swapchain->swapChainExtent.width),
                -static_cast<float>(_swapchain->swapChainExtent.height),
                0.0f,
                1.0f
            )
        );

        cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchain->swapChainExtent));

        uint32_t lastPipeline = UINT32_MAX;

        for (uint32_t i = 0; i < items.size(); i++)
        {
            const RenderItem& item = items[i];
            const PipelineInstance& pipeline = _pipelineManager->getPipeline(item.pipelineID);

            if (item.pipelineID != lastPipeline)
            {
                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline.pipelineLayout, 0, *globalDescriptorSets[frameIndex], nullptr);
                lastPipeline = item.pipelineID;
            }

            ObjectPushConstants push{ .objectIndex = i };
            cmd.pushConstants(*pipeline.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectPushConstants), &push);

            for (const auto& meshItem : item.meshes)
            {
                const Mesh& mesh = _resourceManager->getMesh(meshItem.meshID);

                cmd.bindVertexBuffers(0, vk::Buffer(mesh.vertexBuffer.buffer), { 0 });
                cmd.bindIndexBuffer(vk::Buffer(mesh.indexBuffer.buffer), 0, vk::IndexType::eUint32);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline.pipelineLayout, 1, getMaterialSet(item.pipelineID, meshItem.material), nullptr);
                cmd.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
            }
        }

        cmd.endRendering();

        if (showEditor)
        {
            vk::RenderingAttachmentInfo imguiColorAttachment = {
                .imageView = *_swapchain->swapChainImageViews[imageIndex],
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eLoad,
                .storeOp = vk::AttachmentStoreOp::eStore
            };

            vk::RenderingInfo imguiRenderingInfo = {
                .renderArea = { .offset = { 0, 0 }, .extent = _swapchain->swapChainExtent },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &imguiColorAttachment
            };

            cmd.beginRendering(imguiRenderingInfo);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
            cmd.endRendering();
        }

        transition_image_layout(
            cmd,
            _swapchain->swapChainImages[imageIndex],
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        cmd.end();
    }

    float Renderer::aspectRatio() const
    {
        return static_cast<float>(_swapchain->swapChainExtent.width)
            / static_cast<float>(_swapchain->swapChainExtent.height);
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
        vk::Image image,
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
            .image = image,
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
