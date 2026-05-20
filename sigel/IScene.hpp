#pragma once

#include "GameObject.hpp"
#include "ResourceManager.hpp"
#include "Pipeline.hpp"
#include "Input.hpp"
#include "Camera.hpp"

namespace sigel
{
    struct SceneObject
    {
        uint32_t  pipelineID;
        uint32_t  meshID;
        uint32_t  textureID;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    class IScene {
        public:
        virtual ~IScene() = default;
        
        virtual const std::vector<SceneObject>& getObjects() const = 0;
        virtual const Camera& getCamera() const = 0;
        
        virtual void onEnter(ResourceManager&, PipelineManager&) = 0;
        virtual void onExit(ResourceManager&, PipelineManager&) = 0;
        virtual void onUpdate(float deltaTime) = 0;

        virtual void processInput(const MovementInput &input, float dt) = 0;
    };
}