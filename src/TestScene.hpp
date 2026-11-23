#pragma once

#include <mvel/Scene.hpp>

namespace factory
{
    using namespace mvel;

    class TestScene : public Scene
    {
        public:
            void onEnter() override;
            void onExit() override;
            void onSetup() override;
            void onUpdate(float dt) override;
    };
}
