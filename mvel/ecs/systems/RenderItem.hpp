#pragma once

#include <span>
#include <glm/glm.hpp>
#include "../Entity.hpp"
#include "../components/RenderComponent.hpp"

namespace mvel
{
    struct RenderItem
    {
        Entity entity;
        glm::mat4 modelMatrix;
        uint32_t pipelineID;
        uint32_t modelID;
        // std::span<const MeshItem> meshes;
    };
}
