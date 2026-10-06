#pragma once

#include "../SystemManager.hpp"
#include "../components/RenderComponent.hpp"
#include "../Coordinator.hpp"
#include "RenderItem.hpp"

namespace sigel
{
    class RenderSystem : public System
    {
        public:
            void Update(Coordinator& coordinator);
            const std::vector<RenderItem>& items() const { return renderItems; }

        private:
            std::vector<RenderItem> renderItems;
    };
}
