#pragma once

#include "Transform.hpp"

namespace sigel
{
    struct SceneObject
    {
        uint32_t  pipelineID;
        uint32_t  meshID;
        uint32_t  textureID;
        Transform transform;
    };
}