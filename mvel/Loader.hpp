#pragma once

#include "Material.hpp"
#include <mvel/ecs/components/RenderComponent.hpp>
#include <string>

namespace mvel
{
   // std::vector<SubMesh> loadTinyModel(const std::string &path);
   std::vector<MeshItem> loadAssimpModel(const std::string &path);
}
