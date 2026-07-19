#pragma once

#include "vkapi/Buffer.h"

namespace sigel
{
    struct Mesh
    {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        uint32_t indexCount = 0;
    };
}