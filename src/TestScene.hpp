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
            const Camera& getCamera()  const override { return camera; }

            void onEnter(ResourceManager& rm, PipelineManager& pm) override;
            void onExit(ResourceManager&, PipelineManager&) override;
            void onUpdate(float dt) override;

            void processInput(const MovementInput &input, float dt) override;
            void keyCallback(int key, int scancode, int action, int mods) override;
            void mouseCallback(float dx, float dy) override;
    };
}