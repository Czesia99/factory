#pragma once

#include "GameObject.hpp"
#include "vkapi/ResourceManager.hpp"
#include "vkapi/Pipeline.hpp"
#include "InputManager.hpp"
#include "Camera.hpp"
#include "Transform.hpp"

namespace sigel
{
    struct SceneObject
    {
        uint32_t  pipelineID;
        uint32_t  meshID;
        uint32_t  textureID;
        Transform transform;
    };

    class IScene {
        public:
        virtual ~IScene() = default;
        
        virtual const std::vector<SceneObject>& getObjects() const = 0;
        virtual const Camera& getCamera() const = 0;
        
        virtual void onEnter(ResourceManager&, PipelineManager&) = 0;
        virtual void onExit(ResourceManager&, PipelineManager&) = 0;
        virtual void onUpdate(float deltaTime) = 0;
    };
}