#include "global.h"
#include "levelmodel.h"

std::shared_ptr<LevelModel> g_level = {};
std::shared_ptr<ObjectModel> g_selectedObj = nullptr;
VisualizationModel g_viz = {};

bool g_showGravRanges;
std::string g_jsonFilename;

AssetMan g_assetMan;
std::shared_ptr<Texture> g_gravRangeTex;