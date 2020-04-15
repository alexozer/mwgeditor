#include "global.h"
#include "levelmodel.h"

std::shared_ptr<LevelModel> g_level = {};
std::shared_ptr<ObjectModel> g_selectedObj = nullptr;
VisualizationModel g_viz = {};

Texture g_gravRangeTex;
bool g_showGravRanges;
std::string g_jsonFilename;