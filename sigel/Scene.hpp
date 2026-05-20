#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "IScene.hpp"

namespace sigel
{
    class DefaultScene : public IScene {
        std::vector<SceneObject> objects;
        Camera camera;
        float elapsed = 0.0f;
    public:
        const std::vector<SceneObject>& getObjects() const override { return objects; }
        const Camera& getCamera()  const override { return camera; }

        void onEnter(ResourceManager& rm, PipelineManager& pm) override;
        void onExit(ResourceManager&, PipelineManager&) override;
        void onUpdate(float dt) override;

        void processInput(const MovementInput &input, float dt) override;
        void mouseCallback(float dx, float dy) override;
    };
}