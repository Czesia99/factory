#pragma once

#include "IScene.hpp"

namespace sigel
{
    class DefaultScene : public IScene {
        std::vector<SceneObject> objects;
        Camera camera;
        DirLight sun;
        float elapsed = 0.0f;
    public:
        const std::vector<SceneObject>& getObjects() const override { return objects; }
        Camera& getCamera() override { return camera; }
        DirLight &getLight() override { return sun; }

        void onSetup() override;
        void onEnter() override;
        void onExit() override;
        void onUpdate(float dt) override;
        void onDestroy() override;
    };
}
