#pragma once

#include <glm/glm.hpp>

namespace sigel
{
    struct alignas(16) DirLight 
    {
        glm::vec4 direction{-0.42f, 0.42f, -0.8f, 0.0f};
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f}; 
        glm::vec4 ambient{0.03f, 0.03f, 0.03f, 0.0f};       
    };
}