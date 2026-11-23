#pragma once

#include "Transform.hpp"

namespace mvel
{
    struct Material
    {
        uint32_t  diffuseID;
        uint32_t  metallicID;
        uint32_t  roughnessID;
        uint32_t  normalID;
    };
}
