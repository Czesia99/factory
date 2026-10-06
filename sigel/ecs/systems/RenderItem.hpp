#pragma once

#include <span>
#include <glm/glm.hpp>
#include "../Entity.hpp"
#include "../components/RenderComponent.hpp"

namespace sigel
{
    struct RenderItem
    {
        Entity entity;
        glm::mat4 model;
        uint32_t pipelineID;
        std::span<const MeshItem> meshes;
    };
}
