#include "RenderSystem.hpp"

namespace mvel
{
    void RenderSystem::Update(Coordinator& coordinator)
    {
        renderItems.clear();
        renderItems.reserve(mEntities.size());

        for (Entity entity : mEntities)
        {
            const auto& transform = coordinator.GetComponent<Transform>(entity);
            const auto& render = coordinator.GetComponent<RenderComponent>(entity);

            renderItems.push_back({
                .entity = entity,
                .modelMatrix = transform.getModelMatrix(),
                .pipelineID = render.pipelineID,
                .modelID = render.modelID
            });
        }
    }
}
