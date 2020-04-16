#pragma once

#include "levelmodel.h"
#include <string>

void saveJsonLevel(const std::string& filename, const std::shared_ptr<LevelModel>& level);
std::string validateLevelForExport();

