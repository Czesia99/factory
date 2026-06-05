#include "InputManager.hpp"
#include <iostream>
#include "SigelEngine.hpp"

namespace sigel
{
    InputManager::InputManager()
    {
        // double x, y;
        // glfwGetCursorPos(SigelEngine::get().window, &x, &y);
        // mouse_x = x;
        // mouse_y = y;
    };

    void InputManager::update()
    {
        prevKeys = currKeys;
        mouse_dx = 0.0f;
        mouse_dy = 0.0f;
    }

    void InputManager::onKey(int key, int action)
    {
        if (action == GLFW_PRESS) currKeys.insert(key);
        if (action == GLFW_RELEASE) currKeys.erase(key);
    }

    void InputManager::onMouseMove(float x, float y)
    {
        if (firstMouse) {
            mouse_x = x;
            mouse_y = y;
            firstMouse = false;
            return;
        }
        mouse_dx += x - mouse_x;
        mouse_dy += y - mouse_y;
        mouse_x = x;
        mouse_y = y;
    }
}