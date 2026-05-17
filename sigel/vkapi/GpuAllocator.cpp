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

    AllocatedImage GpuAllocator::createImageTexture(Buffer &imgBuffer, uint32_t width, uint32_t height, VkFormat format) 
    {
        AllocatedImage result;

        VkDeviceSize imageSize = width * height * 4;

        VkImageCreateInfo imageInfo{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = format,
            .extent        = { width, height, 1 },
            .mipLevels     = 1,
            .arrayLayers   = 1,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
            };
            VkDependencyInfo dep1{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &toTransfer };
            vkCmdPipelineBarrier2(*cmd, &dep1);

            // copy buffer → image
            VkBufferImageCopy region{
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                .imageOffset       = { 0, 0, 0 },
                .imageExtent       = { (uint32_t)width, (uint32_t)height, 1 }
            };
            vkCmdCopyBufferToImage(*cmd, imgBuffer.buffer, result.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            // transfer dst → shader read
            VkImageMemoryBarrier2 toShader{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask     = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask    = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .image            = result.image,
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
            };
            VkDependencyInfo dep2{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &toShader };
            vkCmdPipelineBarrier2(*cmd, &dep2);
        });

        VkImageViewCreateInfo imageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = result.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageInfo.format,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
        };

        vkCreateImageView(*_device->logicalDevice, &imageViewInfo, nullptr, &result.view);

        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 8.0f,
            .maxLod = 1.0f,
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

    void GpuAllocator::uploadVertex(const std::vector<Vertex> &vertices, Mesh &mesh)
    {
        vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

        Buffer staging = createStagingBuffer(size);
        memcpy(staging.mapped, vertices.data(), size);

        mesh.vertexBuffer = createBuffer(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = size };
            cmd.copyBuffer(staging.buffer, mesh.vertexBuffer.buffer, region);
        });

        destroyBuffer(staging);
    }

    void GpuAllocator::uploadIndices(const std::vector<uint32_t> &indices, Mesh &mesh)
    {
        vk::DeviceSize size = sizeof(indices[0]) * indices.size();

        Buffer staging = createStagingBuffer(size);
        memcpy(staging.mapped, indices.data(), size);

        mesh.indexBuffer = createBuffer(
            size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );

        immediateSubmit([&](vk::raii::CommandBuffer& cmd) {
            vk::BufferCopy region{ .size = size };
            cmd.copyBuffer(staging.buffer, mesh.indexBuffer.buffer, region);
        });

        destroyBuffer(staging);
    }

    void GpuAllocator::cleanup()
    {
        transferPool.clear();
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }
}