#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "IScene.hpp"
namespace sigel
{
    class DefaultScene : public IScene {
        std::vector<SceneObject> objects;
        float elapsed = 0.0f;
        // Camera camera;
    public:
        const std::vector<SceneObject>& getObjects() const override { return objects; }
        // const Camera&                   getCamera()  const override { return camera; }

        void onEnter(ResourceManager& rm, PipelineManager& pm) override;
        void onExit(ResourceManager&, PipelineManager&) override;
        void onUpdate(float dt) override;
    };
}