#pragma once

#include "assetman.h"
#include "levelmodel.h"
#include "vizmodel.h"

#include <memory>

extern std::shared_ptr<LevelModel> g_level;
extern std::shared_ptr<ObjectModel> g_selectedObj;
extern VisualizationModel g_viz;

extern bool g_showGravRanges;
extern std::string g_jsonFilename;

extern AssetMan g_assetMan;
extern std::shared_ptr<Texture> g_gravRangeTex;
