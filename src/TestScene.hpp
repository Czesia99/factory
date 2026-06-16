#pragma once

#include "../sigel/IScene.hpp"

namespace factory
{
    using namespace sigel;
    
    class TestScene : public IScene 
    {
        public:
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