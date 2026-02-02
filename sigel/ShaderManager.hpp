#pragma once

#include "LogicalDevice.hpp"

namespace sigel
{
    class ShaderManager
    {

        public:
        private:
            LogicalDevice *_lDevice = nullptr;
        public:
            ShaderManager() = default;
            void init(LogicalDevice *_lDevice);

            [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

    };
}