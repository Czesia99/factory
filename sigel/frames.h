#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sigel
{

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    
    struct FrameData 
    {
        vk::raii::CommandBuffer commandBuffer;
        vk::raii::Fence inFlightFence;
        vk::raii::Semaphore presentSemaphore;

        FrameData() : commandBuffer(nullptr), inFlightFence(nullptr), presentSemaphore(nullptr) {}

        FrameData(FrameData&&) = default;
        FrameData& operator=(FrameData&&) = default;
        FrameData(const FrameData&) = delete;
        FrameData& operator=(const FrameData&) = delete;
    };
}