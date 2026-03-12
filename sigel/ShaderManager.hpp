#pragma once

#include "Device.hpp"

namespace sigel
{
    class ShaderManager
    {

        public:
        private:
            Device *_device = nullptr;
        public:
            ShaderManager() = default;
            void init(Device *device);

            [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

    };
}