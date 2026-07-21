#pragma once

#include "IScene.hpp"

namespace sigel
{
    class DefaultScene : public IScene {
        std::vector<SceneObject> objects;
        Camera camera;
        float elapsed = 0.0f;
    public:
        const std::vector<SceneObject>& getObjects() const override { return objects; }
        Camera& getCamera() override { return camera; }

        void onEnter(ResourceManager& rm, PipelineManager& pm) override;
        void onExit(ResourceManager&, PipelineManager&) override;
        void onUpdate(float dt) override;
    };
}
