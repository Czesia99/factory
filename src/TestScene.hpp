#pragma once

#include <sigel/Scene.hpp>

namespace factory
{
    using namespace sigel;

    class TestScene : public Scene
    {
        public:
            void onEnter() override;
            void onExit() override;
            void onSetup() override;
            void onUpdate(float dt) override;
    };
}
