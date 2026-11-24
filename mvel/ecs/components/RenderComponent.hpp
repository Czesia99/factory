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
        uint32_t modelID;
        //std::vector<MeshItem> meshes;
    };
}
