#pragma once 

#include <GLFW/glfw3.h>
#include <unordered_set>

namespace sigel
{
    class InputManager
    {
        public:
            InputManager();
            void update();
            void onKey(int key, int action);
            void onMouseMove(float dx, float dy);

            bool isHeld (int key) const { return currKeys.count(key); }
            bool isPressed (int key) const { return currKeys.count(key) && !prevKeys.count(key); }
            bool isReleased (int key) const { return !currKeys.count(key) && prevKeys.count(key); }

            float getMouseDeltaX() const { return mouse_dx; }
            float getMouseDeltaY() const { return mouse_dy; }

        private:
        public:
        private:
            std::unordered_set<int> currKeys, prevKeys;
            float mouse_x = 0.0f;
            float mouse_y = 0.0f;
            float mouse_dx = 0.0f;
            float mouse_dy = 0.0f;

            bool firstMouse = true;

    };
}

//https://gamedev.stackexchange.com/questions/150157/how-to-improve-my-input-handling-in-glfw