#pragma once

#include "imgui.h"
#include "textures.h"

#include <vector>

enum class PlanetOrder { START, MIDDLE, END };

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
    PlanetOrder order;
};

struct FoodModel : public ObjectModel
{
    bool cookable;
    int cols;
};

struct LevelModel {
    int levelNumber;

    std::vector<PlanetModel> planets;

    std::vector<FoodModel> foods;

    ObjectModel player;
    ObjectModel customer;
};

