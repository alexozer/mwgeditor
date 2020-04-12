#pragma once

#include "imgui.h"
#include "textures.h"

#include <vector>
#include <memory>

enum class PlanetOrder { START, MIDDLE, END };

struct ObjectModel
{
    Texture tex;
    float scale;
    ImVec2 pos;
    ImVec2 anchor;

    int cols; // Columns of animation frames
    int span; // Total animation frames
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
};

struct LevelModel {
    int levelNumber;

    std::vector<std::shared_ptr<PlanetModel>> planets;

    std::vector<std::shared_ptr<FoodModel>> foods;

    std::shared_ptr<ObjectModel> player;
    std::shared_ptr<ObjectModel> customer;
};

