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
        mousedx = 0.0f;
        mousedy = 0.0f;
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
        mousedx += x - mouse_x;
        mousedy += y - mouse_y;
        mouse_x = x;
        mouse_y = y;
    }
}