#pragma once

#include "Transform.hpp"

namespace sigel
{
    struct MeshInfo
    {
        uint32_t  meshID;
        uint32_t  textureID;
        //localtransform
    };

    struct SceneObject
    {
        uint32_t  pipelineID;
        std::vector<MeshInfo> meshes;
        Transform transform;
    };
}