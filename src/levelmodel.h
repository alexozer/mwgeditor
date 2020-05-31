#pragma once

#include "imgui.h"
#include "assetman.h"

#include <vector>
#include <memory>

enum class PlanetOrder { START, MIDDLE, END };
enum class PlanetType { NORMAL, SUN, BLACKHOLE, STORAGE, SEASON };

struct ObjectModel
{
    std::shared_ptr<Texture> tex;
    float scale;
    ImVec2 pos;
    ImVec2 anchor;

    int cols; // Columns of animation frames
    int span; // Total animation frames

    inline ImVec2 frameSize()
    {
        int rows = span / cols;
        if (span % cols != 0) rows++;
        return ImVec2(static_cast<float>(tex->width) / cols,
                      static_cast<float>(tex->height) / rows);
    }

    inline ImVec2 uvEnd()
    {
        int rows = span / cols;
        if (span % cols != 0) rows++;
        return ImVec2(1.f / cols, 1.f / rows);
    }

    virtual ~ObjectModel() = default;
};

struct PlanetModel : public ObjectModel
{
    bool hasFood;
    PlanetType type;
    PlanetOrder order;
};

struct FoodModel : public ObjectModel
{
    bool cookable;
    bool seasonable;
};

struct LevelModel {
    int levelNumber;
    float levelTimer;

    std::vector<std::shared_ptr<PlanetModel>> planets;

    std::vector<std::shared_ptr<FoodModel>> foods;

    std::shared_ptr<ObjectModel> player;
    std::shared_ptr<ObjectModel> customer;
};

