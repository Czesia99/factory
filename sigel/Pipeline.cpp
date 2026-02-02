#include "Pipeline.hpp"
#include "Utils.hpp"

namespace sigel
{
    void Pipeline::init(ShaderManager *shaderManager)
    {
        _shaderManager = shaderManager;
    }

    void Pipeline::createGraphicsPipeline(const std::vector<char> &shaderCode)
    {
        vk::raii::ShaderModule shaderModule = _shaderManager->createShaderModule(shaderCode);
    }
}