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

    void Renderer::loadObject(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t pipelineID, Material &mat)
    {
        RenderObject object;

        object.pipelineID = pipelineID;
        for (auto &mesh : object.meshes)
        {
            mesh.meshID = _resourceManager->createMesh(vertices, indices);
            mesh.diffuseID = mat.diffuseID;
            mesh.metallicID = mat.metallicID;
            mesh.roughnessID = mat.roughnessID;
            mesh.normalID = mat.normalID;
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            object.uniformBuffers.emplace_back(_resourceManager->createUniformBuffer(sizeof(UniformBufferObject)));
        }
        renderObjects.emplace_back(std::move(object));
    }

    void Renderer::prepareScene(const Scene& scene)
    {
        cleanupRenderObjects();
        descriptorPool.clear();

        const auto& sceneObjects = scene.getObjects();
        if (sceneObjects.empty()) return;

        for (const auto& so : sceneObjects) {
            RenderObject ro;
            ro.pipelineID = so.pipelineID;
            for (const auto &mesh : so.meshes)
            {
                MeshRenderData renderMesh;

                renderMesh.meshID = mesh.meshID;
                renderMesh.diffuseID = mesh.material.diffuseID;
                renderMesh.metallicID = mesh.material.metallicID;
                renderMesh.roughnessID = mesh.material.roughnessID;
                renderMesh.normalID = mesh.material.normalID;
                ro.meshes.emplace_back(std::move(renderMesh));
            }

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

            for (auto &mesh : obj.meshes)
            {
                mesh.descriptorSets.clear();
            }
        }
        renderObjects.clear();
    }

    void Renderer::drawFrame(Scene& scene, bool showEditor)
    {
        auto &frame = currentFrame();

        if (_pipelineManager->msaaChanged)
        {
            _device->logicalDevice.waitIdle();
            _swapchain->recreateSwapChain();
            _pipelineManager->recreateAllPipelines();
            _pipelineManager->msaaChanged = false;
        }

        waitFence();

		auto [result, imageIndex] = _swapchain->swapChain.acquireNextImage(UINT64_MAX, *frame.presentSemaphore, nullptr);

        checkImageResult(result);

        _device->logicalDevice.resetFences(*frame.inFlightFence);

        updateUniformBuffer(frameIndex, scene);
        frame.commandBuffer.reset();
        recordCommandBuffer(imageIndex, showEditor);

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
        uint32_t totalSubMeshes = 0;
        for (const auto& ro : renderObjects)
        {
            totalSubMeshes += static_cast<uint32_t>(ro.meshes.size());
        }

        if (totalSubMeshes == 0) return;

        uint32_t maxSets = totalSubMeshes * MAX_FRAMES_IN_FLIGHT;

        std::array poolSize {
            vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, maxSets),
            vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, maxSets * 4),
        };

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets       = maxSets,
            .poolSizeCount = poolSize.size(),
            .pPoolSizes    = poolSize.data()
        };

        descriptorPool = vk::raii::DescriptorPool(_device->logicalDevice, poolInfo);
    }

    void Renderer::createDescriptorSets()
    {
        for (auto& obj : renderObjects)
        {
            const PipelineInstance& pipeline =
                _pipelineManager->getPipeline(obj.pipelineID);

            std::vector<vk::DescriptorSetLayout> layouts(
                MAX_FRAMES_IN_FLIGHT,
                *pipeline.descriptorSetLayout
            );

            vk::DescriptorSetAllocateInfo allocInfo{
                .descriptorPool = *descriptorPool,
                .descriptorSetCount =
                    static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            for (auto &mesh : obj.meshes)
            {
                mesh.descriptorSets = _device->logicalDevice.allocateDescriptorSets(allocInfo);

                auto getTexInfo = [&](uint32_t texID) -> vk::DescriptorImageInfo {
                    const auto& tex = _resourceManager->textures[texID];
                    return vk::DescriptorImageInfo{
                            .sampler = tex.sampler,
                            .imageView = tex.view,
                            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
                };

                vk::DescriptorImageInfo diffuseInfo   = getTexInfo(mesh.diffuseID);
                vk::DescriptorImageInfo metallicInfo  = getTexInfo(mesh.metallicID);
                vk::DescriptorImageInfo roughnessInfo = getTexInfo(mesh.roughnessID);
                vk::DescriptorImageInfo normalInfo    = getTexInfo(mesh.normalID);

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                {
                    vk::DescriptorBufferInfo bufferInfo{
                        .buffer = vk::Buffer(
                            obj.uniformBuffers[i].buffer
                        ),
                        .offset = 0,
                        .range = sizeof(UniformBufferObject)
                    };

                    std::array<vk::WriteDescriptorSet, 5> descriptorWrites{{
                    {
                        .dstSet = *mesh.descriptorSets[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                        .pBufferInfo = &bufferInfo
                    },
                    {
                        .dstSet = *mesh.descriptorSets[i],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &diffuseInfo
                    },
                    {
                        .dstSet = *mesh.descriptorSets[i],
                        .dstBinding = 2,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &metallicInfo
                    },
                    {
                        .dstSet = *mesh.descriptorSets[i],
                        .dstBinding = 3,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &roughnessInfo
                    },
                    {
                        .dstSet = *mesh.descriptorSets[i],
                        .dstBinding = 4,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &normalInfo
                    }
                        }};

                    _device->logicalDevice.updateDescriptorSets(
                        descriptorWrites,
                        nullptr
                    );
                }
            }
        }
    }

    FrameData& Renderer::currentFrame()
    {
        return frames[frameIndex];
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage, Scene& scene)
    {
        const auto& sceneObjects = scene.getObjects();
        const auto& sceneCamera = scene.getCamera();
        const auto& sceneDirLight = scene.getLight();

        float width  = static_cast<float>(_swapchain->swapChainExtent.width);
        float height = static_cast<float>(_swapchain->swapChainExtent.height);
        float aspect = width / height;

        UniformBufferObject ubo{};
        ubo.view  = sceneCamera.getViewMatrix();
        ubo.proj  = sceneCamera.getProjectionMatrix(aspect);
        ubo.light = sceneDirLight;
        ubo.camPos = sceneCamera.cam.pos;

        for (size_t i = 0; i < renderObjects.size(); i++) {
            ubo.model = sceneObjects[i].transform.getModelMatrix();
            memcpy(renderObjects[i].uniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));
        }
    }

    void Renderer::updateLightBuffer(uint32_t currentImage, Scene& scene)
    {
        const auto& sceneDirLight = scene.getLight();

        float width  = static_cast<float>(_swapchain->swapChainExtent.width);
        float height = static_cast<float>(_swapchain->swapChainExtent.height);
        float aspect = width / height;

            LightBufferObject lbo{};
            lbo.light = sceneDirLight;
            // memcpy(, &lbo, sizeof(lbo));
    }

    void Renderer::recordCommandBuffer(uint32_t imageIndex, bool showEditor)
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

        for (const auto& renderObject : renderObjects)
        {
            const PipelineInstance& pipeline = _pipelineManager->getPipeline(renderObject.pipelineID);
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);

            for (const auto& meshData : renderObject.meshes)
            {
                const Mesh& mesh = _resourceManager->getMesh(meshData.meshID);

                vk::Buffer vb = mesh.vertexBuffer.buffer;
                vk::Buffer ib = mesh.indexBuffer.buffer;

                cmd.bindVertexBuffers(0, vb, { 0 });
                cmd.bindIndexBuffer(ib, 0, vk::IndexType::eUint32);

                cmd.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    *pipeline.pipelineLayout,
                    0,
                    *meshData.descriptorSets[frameIndex],
                    nullptr
                );

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
