#include "GpuAllocator.hpp"

namespace sigel
{
    void GpuAllocator::init(Device *device, Instance *instance)
    {
        _device = device;

        VmaAllocatorCreateInfo allocatorInfo{
            .physicalDevice = *device->physicalDevice,
            .device         = *device->logicalDevice,
            .instance       = *instance->instance,
        };

        vmaCreateAllocator(&allocatorInfo, &allocator);

        vk::CommandPoolCreateInfo poolInfo{
            .flags            = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = device->graphicsIndex
        };
        transferPool = vk::raii::CommandPool(device->logicalDevice, poolInfo);
    }

    Buffer GpuAllocator::createBuffer(vk::DeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage)
    {
        Buffer result;

        VkBufferCreateInfo bufferInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocInfo{
            .usage = memoryUsage
        };

        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, nullptr);

        return result;
    }

    Buffer GpuAllocator::createStagingBuffer(vk::DeviceSize size)
    {
        Buffer result;

        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        VmaAllocationCreateInfo allocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo info{};
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &info);
        result.mapped = info.pMappedData;
        return result;
    }

    Buffer GpuAllocator::createUniformBuffer(vk::DeviceSize size)
    {
        Buffer result;

        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        VmaAllocationCreateInfo allocInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo info{};
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &info);
        result.mapped = info.pMappedData;
        return result;
    }

    void GpuAllocator::destroyBuffer(Buffer& buffer)
    {
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
        buffer.buffer     = VK_NULL_HANDLE;
        buffer.allocation = VK_NULL_HANDLE;
        buffer.mapped     = nullptr;
    }

    AllocatedImage GpuAllocator::createDepthImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
    {
        AllocatedImage result;

        VkImageCreateInfo imageInfo{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = format,
            .extent        = { width, height, 1 },
            .mipLevels     = 1,
            .arrayLayers   = 1,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = usage,
            .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocInfo{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };

        vmaCreateImage(allocator, &imageInfo, &allocInfo, &result.image, &result.allocation, nullptr);

        return result;
    }

    AllocatedImage GpuAllocator::createImageTexture(Buffer &imgBuffer, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format)
    {
        AllocatedImage result;

        VkDeviceSize imageSize = width * height * 4;

        VkImageCreateInfo imageInfo{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = format,
            .extent        = { width, height, 1 },
            .mipLevels     = mipLevels,
            .arrayLayers   = 1,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocInfo{ .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE };
        vmaCreateImage(allocator, &imageInfo, &allocInfo, &result.image, &result.allocation, nullptr);

        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            VkImageMemoryBarrier2 toTransfer{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = VK_PIPELINE_STAGE_2_NONE,
                .srcAccessMask    = VK_ACCESS_2_NONE,
                .dstStageMask     = VK_PIPELINE_STAGE_2_COPY_BIT,
                .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = result.image,
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 }
            };

            VkDependencyInfo dep1{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &toTransfer };
            vkCmdPipelineBarrier2(*cmd, &dep1);

            VkBufferImageCopy region{
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                .imageOffset       = { 0, 0, 0 },
                .imageExtent       = { (uint32_t)width, (uint32_t)height, 1 }
            };

            vkCmdCopyBufferToImage(*cmd, imgBuffer.buffer, result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            generateMipmaps(cmd, result.image, format, width, height, mipLevels);
        });

        VkImageViewCreateInfo imageViewInfo {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = result.image,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = imageInfo.format,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = mipLevels, .layerCount = 1 }
        };

        vkCreateImageView(*_device->logicalDevice, &imageViewInfo, nullptr, &result.view);

        VkSamplerCreateInfo samplerInfo {
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter        = VK_FILTER_LINEAR,
            .minFilter        = VK_FILTER_LINEAR,
            .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		    .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		    .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias       = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy    = 8.0f,
		    .compareEnable    = VK_FALSE,
		    .compareOp        = VK_COMPARE_OP_ALWAYS,
		    .minLod           = 0.0f,
		    .maxLod           = VK_LOD_CLAMP_NONE,
        };

        vkCreateSampler(*_device->logicalDevice, &samplerInfo, nullptr, &result.sampler);

        return result;
    }

    void GpuAllocator::destroyImage(AllocatedImage& image)
    {
        VmaAllocatorInfo info{};
        vmaGetAllocatorInfo(allocator, &info);

        if (image.sampler != VK_NULL_HANDLE)
            vkDestroySampler(info.device, image.sampler, nullptr);

        if (image.view != VK_NULL_HANDLE)
            vkDestroyImageView(info.device, image.view, nullptr);

        vmaDestroyImage(allocator, image.image, image.allocation);
        image.image      = VK_NULL_HANDLE;
        image.view = VK_NULL_HANDLE;
        image.sampler = VK_NULL_HANDLE;
        image.allocation = VK_NULL_HANDLE;
    }

    void GpuAllocator::immediateSubmit(std::function<void(vk::raii::CommandBuffer&)> fn)
    {
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool        = *transferPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto cmd = std::move(
            vk::raii::CommandBuffers(_device->logicalDevice, allocInfo).front()
        );

        cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        fn(cmd);
        cmd.end();

        vk::SubmitInfo submitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers    = &*cmd
        };

        _device->graphicsQueue.submit(submitInfo);
        _device->graphicsQueue.waitIdle();
    }

    void GpuAllocator::uploadVertex(const std::vector<Vertex> &vertices, Buffer &buffer)
    {
        vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

        Buffer staging = createStagingBuffer(size);
        memcpy(staging.mapped, vertices.data(), size);

        buffer = createBuffer(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = size };
            cmd.copyBuffer(staging.buffer, buffer.buffer, region);
        });

        destroyBuffer(staging);
    }

    void GpuAllocator::uploadIndices(const std::vector<uint32_t> &indices, Buffer &buffer)
    {
        vk::DeviceSize size = sizeof(indices[0]) * indices.size();

        Buffer staging = createStagingBuffer(size);
        memcpy(staging.mapped, indices.data(), size);

        buffer = createBuffer(
            size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = size };
            cmd.copyBuffer(staging.buffer, buffer.buffer, region);
        });

        destroyBuffer(staging);
    }

    void GpuAllocator::cleanup()
    {
        transferPool.clear();
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

    void GpuAllocator::generateMipmaps(vk::raii::CommandBuffer& cmd, VkImage image, VkFormat imageFormat, uint32_t width, uint32_t height, uint32_t mipLevels)
    {
        vk::FormatProperties formatProperties = _device->physicalDevice.getFormatProperties(static_cast<vk::Format>(imageFormat));

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        int32_t mipWidth  = width;
        int32_t mipHeight = height;

        vk::ImageMemoryBarrier barrier = {
            .srcAccessMask       = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask       = vk::AccessFlagBits::eTransferRead,
            .oldLayout           = vk::ImageLayout::eTransferDstOptimal,
            .newLayout           = vk::ImageLayout::eTransferSrcOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image,
            .subresourceRange    = {
                .aspectMask     = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };

        for (uint32_t i = 1; i < mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

            vk::ImageBlit blit = {
                .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i - 1, .baseArrayLayer = 0, .layerCount = 1},
                .srcOffsets     = std::array<vk::Offset3D, 2>({vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth, mipHeight, 1}}),
                .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i, .baseArrayLayer = 0, .layerCount = 1},
                .dstOffsets     = std::array<vk::Offset3D, 2>({vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}})
            };

            cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

            barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

            if (mipWidth > 1) { mipWidth /= 2; }
            if (mipHeight > 1) { mipHeight /= 2; }
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
    }
}
