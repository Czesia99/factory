#pragma once

#include <glm/glm.hpp>

namespace sigel
{
    enum LightType
    {
        Point,
        Spot,
        Directional
    };

    struct LightComponent {
        LightType type = LightType::Point;
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec4 ambient = glm::vec4(0.03f);
        float intensity = 1.0f;
        float cutOff = 12.5f;
        float outerCutOff = 17.5f;
    };

    struct alignas(16) DirLight
    {
        glm::vec4 direction{-0.42f, 0.42f, -0.8f, 0.0f};
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 ambient{1.0f, 1.0f, 1.0f, 0.05f};
    };
}
