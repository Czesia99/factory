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

            // virtual void onSetup(ResourceManager&, PipelineManager&) = 0;
            virtual void onEnter(ResourceManager&, PipelineManager&) = 0;
            virtual void onExit(ResourceManager&, PipelineManager&) = 0;
            virtual void onUpdate(float deltaTime) = 0;
            // virtual void onDestroy() = 0;
    };
}
