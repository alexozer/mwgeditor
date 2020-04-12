#pragma once

#include "imgui.h"
#include "textures.h"

#include <vector>

struct ObjectModel
{
    Texture tex;
    float scale;
    ImVec2 pos;
    ImVec2 anchor;
};

struct PlanetModel : public ObjectModel
{
    bool isSun;
    bool hasFood;
};

struct FoodModel : public ObjectModel
{
    bool cookable;
    int cols;
};

struct LevelModel {
    int levelNumber;

    PlanetModel startPlanet;
    PlanetModel endPlanet;
    std::vector<PlanetModel> planets;

    std::vector<FoodModel> foods;

    ObjectModel player;
    ObjectModel customer;
};

