#include "util.h"

std::vector<std::shared_ptr<ObjectModel>> getAllLevelObjects(const std::shared_ptr<LevelModel> &level)
{
    std::vector<std::shared_ptr<ObjectModel>> allObjects;
    allObjects.insert(allObjects.end(), level->planets.begin(), level->planets.end());
    allObjects.insert(allObjects.end(), level->foods.begin(), level->foods.end());
    if (level->customer) allObjects.emplace_back(level->customer);
    if (level->player) allObjects.emplace_back(level->player);
    return allObjects;
}
