#include "loadjson.h"
#include "global.h"

#include "json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;
namespace fs = std::filesystem;

template <typename T>
static T getJsonDefault(const json& j, const std::string& key, T def)
{
    auto it = j.find(key);
    if (it != j.end()) {
        return it->get<T>();
    }
    return def;
}

void loadJsonAssets()
{
    std::ifstream f(g_assetMan.getAssetPathRoot() / "json" / "assets.json");
    json assetsJson;
    f >> assetsJson;

    for (auto& texJsonItem : assetsJson["textures"].items())
    {
        const std::string& name = texJsonItem.key();
        if (!g_assetMan.findTextureByShortName(name))
        {
            fs::path texPath = g_assetMan.getAssetPathRoot() / texJsonItem.value()["file"].get<std::string>();
            texPath.make_preferred();
            g_assetMan.loadTexture(texPath, name);
        }
    }
}

ImVec2 loadJsonCoord(const json& coordJson)
{
    if (coordJson.is_array())
    {
        auto vec = coordJson.get<std::vector<float>>();
        return ImVec2(vec[0], vec[1]);
    }

    // Scale specified as single number
    float scale = coordJson.get<float>();
    return ImVec2(scale, scale);
}

void loadObjectModel(const json& objectJson, std::shared_ptr<ObjectModel> objectModel)
{
    auto texName = objectJson["data"]["texture"].get<std::string>();
    objectModel->tex = g_assetMan.findTextureByShortName(texName);
    objectModel->pos = loadJsonCoord(objectJson["data"]["position"]);
    objectModel->anchor = loadJsonCoord(objectJson["data"]["anchor"]);
    objectModel->scale = loadJsonCoord(objectJson["data"]["scale"]).x;

    objectModel->pos.y = -objectModel->pos.y; // Flip Y coordinate (little hacky but w/e)

    objectModel->cols = getJsonDefault(objectJson["data"], "cols", 1);
    objectModel->span = getJsonDefault(objectJson["data"], "span", objectModel->cols);
}

std::shared_ptr<PlanetModel> loadJsonPlanet(const json& planetJson)
{
    auto planetModel = std::make_shared<PlanetModel>();

    loadObjectModel(planetJson, planetModel);

    planetModel->hasFood = getJsonDefault(planetJson["data"], "hasFood", false);

    bool isSun = getJsonDefault(planetJson["data"], "isSun", false);
    bool isBlackHole = getJsonDefault(planetJson["data"], "isBlackHole", false);
    bool isStorage = getJsonDefault(planetJson["data"], "isStorage", false);
    bool isSeason = getJsonDefault(planetJson["data"], "isSeasonPlanet", false);

    if (isSun) planetModel->type = PlanetType::SUN;
    else if (isBlackHole) planetModel->type = PlanetType::BLACKHOLE;
    else if (isStorage) planetModel->type = PlanetType::STORAGE;
    else if (isSeason) planetModel->type = PlanetType::SEASON;
    else planetModel->type = PlanetType::NORMAL;

    return planetModel;
}

std::shared_ptr<FoodModel> loadJsonFood(const json& foodJson)
{
    auto foodModel = std::make_shared<FoodModel>();

    loadObjectModel(foodJson, foodModel);

    foodModel->cookable = getJsonDefault(foodJson["data"], "cookable", false);
    foodModel->seasonable = getJsonDefault(foodJson["data"], "seasonable", false);

    return foodModel;
}

std::shared_ptr<LevelModel> loadJsonLevel(const std::string& filename)
{
    loadJsonAssets();

    std::ifstream f(filename);
    json levelJson;
    f >> levelJson;

    auto levelModel = std::make_shared<LevelModel>();

    // Get level number
    std::string levelNumStr = levelJson["scenes"].items().begin().key();
    levelModel->levelNumber = std::stoi(levelNumStr.substr(2, levelNumStr.size()));

    // Load planets
    auto& planetMapJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planets"]["children"];
    for (auto& planetJsonItem : planetMapJson.items())
    {
        auto planet = loadJsonPlanet(planetJsonItem.value());
        if (planetJsonItem.key() == "startPlanet")
        {
            planet->order = PlanetOrder::START;
        }
        else if (planetJsonItem.key() == "endPlanet")
        {
            planet->order = PlanetOrder::END;
        }
        else
        {
            planet->order = PlanetOrder::MIDDLE;
        }
        levelModel->planets.emplace_back(planet);
    }

    // Load foods
    auto& foodMapJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["food"]["children"];
    for (auto& foodJsonItem : foodMapJson.items())
    {
        levelModel->foods.emplace_back(loadJsonFood(foodJsonItem.value()));
    }

    // Load player
    auto& playerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["player"];
    levelModel->player = std::make_shared<ObjectModel>();
    loadObjectModel(playerJson, levelModel->player);

    // Load customer
    auto& customerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["customer"];
    levelModel->customer = std::make_shared<ObjectModel>();
    loadObjectModel(customerJson, levelModel->customer);

    // Load level timer
    auto& timerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["timer"]["data"]["timer"];
    levelModel->levelTimer = timerJson.get<float>();

    return levelModel;
}
