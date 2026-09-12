#pragma once

#include <array>
#include <cassert>
#include <queue>

#include "Entity.hpp"

namespace sigel
{
    class EntityManager
    {
        public:
            EntityManager();
            Entity createEntity();
            void DestroyEntity(Entity entity);
            void SetSignature(Entity entity, Signature signature);
            Signature GetSignature(Entity entity);

        private:
            std::queue<Entity> mAvailableEntities{};
            std::array<Signature, MAX_ENTITIES> mSignatures{};
            uint32_t mLivingEntityCount{};
    };
}
