#include "ShaderManager.hpp"

namespace sigel
{
    void ShaderManager::init(LogicalDevice *lDevice)
    {
        _lDevice = lDevice;
    }

    [[nodiscard]] vk::raii::ShaderModule ShaderManager::createShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
        vk::raii::ShaderModule shaderModule{ _lDevice->getDevice(), createInfo }; //ldevice needed
        return shaderModule;
    }
}