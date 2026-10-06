#pragma once

#include "Material.hpp"
#include <sigel/ecs/components/RenderComponent.hpp>
#include <string>

namespace sigel
{
   // std::vector<SubMesh> loadTinyModel(const std::string &path);
   std::vector<MeshItem> loadAssimpModel(const std::string &path);
}
