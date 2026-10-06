#pragma once

#include "InputManager.hpp"
#include "Camera.hpp"
#include "Material.hpp"
#include "Light.hpp"

namespace sigel
{
    class Scene
    {
        protected:
            Camera camera;
            DirLight dirLight;
            float elapsed = 0.0f;


        public:
            virtual ~Scene() = default;

            virtual Camera& getCamera() { return camera; }
            virtual DirLight& getLight() { return dirLight; }

            virtual void onSetup();
            virtual void onEnter() {}
            virtual void onExit() {}
            virtual void onUpdate(float dt);
            virtual void onDestroy() { /* bjects.clear(); */ }

            bool isSetup = false;
    };

    class DefaultScene : public Scene {};
}
