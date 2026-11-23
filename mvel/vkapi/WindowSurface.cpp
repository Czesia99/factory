#include "WindowSurface.hpp"
#include <mvel/Utils.hpp>

namespace mvel
{
    void WindowSurface::createSurface(vk::raii::Instance &instance, GLFWwindow *window)
    {
        VkSurfaceKHR _surface;

        if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
            throw std::runtime_error("failed to create window surface!");
        }

        surface = vk::raii::SurfaceKHR(instance, _surface);
        status("SURFACE", "Surface created");
    }
}
