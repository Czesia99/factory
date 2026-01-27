#include <vulkan/vulkan_raii.hpp>

namespace sigel
{
    class SigelEngine {
        public:
            SigelEngine() = default;
            void run();
        private:
            void initVulkan();
            void mainLoop();
            void cleanup();
    };
}