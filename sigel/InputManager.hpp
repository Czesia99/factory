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

            bool isHeld (int key) { return currKeys.count(key); }
            bool isPressed (int key) { return currKeys.count(key) && !prevKeys.count(key); }
            bool isReleased (int key) { return !currKeys.count(key) && prevKeys.count(key); }

            float getMouseDeltaX() { return mousedx; }
            float getMouseDeltaY() { return mousedy; }

        private:
        public:
        private:
            std::unordered_set<int> currKeys, prevKeys;
            float mouse_x = 0.0f;
            float mouse_y = 0.0f;
            float mousedx = 0.0f;
            float mousedy = 0.0f;

            bool firstMouse = true;

    };
}

//https://gamedev.stackexchange.com/questions/150157/how-to-improve-my-input-handling-in-glfw