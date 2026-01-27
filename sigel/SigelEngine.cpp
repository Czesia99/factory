#include "SigelEngine.hpp"

namespace sigel
{
    void SigelEngine::run()
    {
        printf("%s", "engine run");
        initVulkan();
        mainLoop();
        cleanup();
    }

    void SigelEngine::initVulkan()
    {

    }

    void SigelEngine::mainLoop()
    {

    }

    void SigelEngine::cleanup()
    {

    }
}