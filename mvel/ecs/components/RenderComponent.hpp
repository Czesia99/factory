#pragma once

#include <cstdint>
#include <vector>

#include <mvel/Material.hpp>
#include <mvel/MeshItem.hpp>

namespace mvel
{
    struct RenderComponent
    {
        uint32_t pipelineID;
        std::vector<MeshItem> meshes;
        //uint32_t modelID;
    };
}
