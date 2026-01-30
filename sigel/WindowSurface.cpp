#include "WindowSurface.hpp"

namespace sigel
{
    void WindowSurface::createSurface(vk::raii::Instance &instance, GLFWwindow *window)
    {
        VkSurfaceKHR _surface;

        if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
            throw std::runtime_error("failed to create window surface!");
        }

        surface = vk::raii::SurfaceKHR(instance, _surface);
    }
}