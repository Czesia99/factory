#pragma once

#include "vkapi/Buffer.h"

namespace mvel
{
    struct Mesh
    {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        uint32_t indexCount = 0;
    };
}
