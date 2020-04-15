#pragma once

#include <memory>
#include "textures.h"
#include "levelmodel.h"
#include "vizmodel.h"

extern std::shared_ptr<LevelModel> g_level;
extern std::shared_ptr<ObjectModel> g_selectedObj;
extern VisualizationModel g_viz;

extern Texture g_gravRangeTex;
extern bool g_showGravRanges;
extern std::string g_jsonFilename;