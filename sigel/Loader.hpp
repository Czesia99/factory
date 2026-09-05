#pragma once

#include "Object.hpp"
#include <iostream>

namespace sigel
{
   std::vector<SubMesh> loadTinyModel(const std::string &path);
   std::vector<SubMesh> loadAssimpModel(const std::string &path);
}
