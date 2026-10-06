#pragma once

#include <cstdint>
#include <vector>

#include <sigel/Material.hpp>

namespace sigel
{
    struct MeshItem
    {
        uint32_t meshID;
        Material material;
    };

    struct RenderComponent
    {
        uint32_t pipelineID;
        std::vector<MeshItem> meshes;
    };
}
