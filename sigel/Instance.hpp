#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class Instance
    {
        private:
            vk::raii::Context  context;
            vk::raii::Instance instance = nullptr;
        public:
            Instance() = default;
            void createInstance();
        private:
    };
}