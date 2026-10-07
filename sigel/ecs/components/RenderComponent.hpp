#pragma once

#include <cstdint>
#include <vector>

#include <sigel/Material.hpp>
#include <sigel/MeshItem.hpp>

namespace sigel
{
    struct RenderComponent
    {
        uint32_t pipelineID;
        std::vector<MeshItem> meshes;
        //uint32_t modelID;
    };
}
