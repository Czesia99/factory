#pragma once

#include "Mesh.hpp"
#include "vkapi/ResourceManager.hpp"
#include "vkapi/Pipeline.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "Object.hpp"

namespace sigel
{
    class IScene {
        public:
            virtual ~IScene() = default;

            virtual const std::vector<SceneObject>& getObjects() const = 0;
            virtual Camera& getCamera() = 0;

            virtual void onSetup() = 0;
            virtual void onEnter() = 0;
            virtual void onExit() = 0;
            virtual void onUpdate(float deltaTime) = 0;
            virtual void onDestroy() = 0;

            bool isSetup = false;
    };
}
