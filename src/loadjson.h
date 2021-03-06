#pragma once

#include "levelmodel.h"
#include <string>

std::shared_ptr<LevelModel> loadJsonLevel(const std::string& filename);