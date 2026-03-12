#include "ShaderManager.hpp"

namespace sigel
{
    void ShaderManager::init(Device *device)
    {
        _device = device;
    }

    [[nodiscard]] vk::raii::ShaderModule ShaderManager::createShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
        vk::raii::ShaderModule shaderModule{ _device->logicalDevice, createInfo };
        return shaderModule;
    }
}