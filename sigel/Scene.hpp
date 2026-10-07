#pragma once

#include <vector>
#include <sigel/ecs/Entity.hpp>
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

        protected:
            Entity createEntity();


        public:
            virtual ~Scene() = default;

            virtual Camera& getCamera() { return camera; }
            virtual DirLight& getLight() { return dirLight; }

            virtual void onSetup();
            virtual void onEnter();
            virtual void onExit() {}
            virtual void onUpdate(float dt);
            virtual void onDestroy() { }

            void destroyEntities();

            bool isSetup = false;
        private:
            std::vector<Entity> entities;
    };

    class DefaultScene : public Scene {};
}
