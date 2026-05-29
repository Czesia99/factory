#pragma once 

#include <unordered_set>

namespace sigel
{
    class InputManager
    {
        public:
            void onKey(int key, int action)
            {
                if (action == GLFW_PRESS) currKeys.insert(key);
                if (action == GLFW_RELEASE) currKeys.erase(key);
            }

            void update()
            {
                prevKeys = currKeys;
            }

            bool isHeld (int key) { return currKeys.count(key); }
            bool isPressed (int key) { return currKeys.count(key) && !prevKeys.count(key); }
            bool isReleased (int key) { return !currKeys.count(key) && prevKeys.count(key); }
        private:
            std::unordered_set<int> currKeys, prevKeys;

        public:
        private:
    };
}

//https://gamedev.stackexchange.com/questions/150157/how-to-improve-my-input-handling-in-glfw