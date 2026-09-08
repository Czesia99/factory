#pragma once

#include "Transform.hpp"

namespace sigel
{
    struct Material
    {
        uint32_t  diffuseID;
        uint32_t  metallicID;
        uint32_t  roughnessID;
        uint32_t  normalID;
    };

    struct SubMesh
    {
        uint32_t  meshID;
        Material material;
        // uint32_t  textureID;
        //localtransform
        //material
    };

    struct SceneObject
    {
        uint32_t  pipelineID;
        std::vector<SubMesh> meshes;
        Transform transform;
    };
}
