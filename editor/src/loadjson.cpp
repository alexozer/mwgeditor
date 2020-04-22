#include "loadjson.h"
#include "global.h"

#include "json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;
namespace fs = std::filesystem;

void loadJsonAssets()
{
    std::ifstream f(AssetMan::getAssetPathRoot() / "json" / "assets_demo.json");
    json assetsJson;
    f >> assetsJson;

    for (auto& texJsonItem : assetsJson["textures"].items())
    {
        const std::string& name = texJsonItem.key();
        if (!g_assetMan.findTextureByShortName(name))
        {
            fs::path texPath = AssetMan::getAssetPathRoot() / texJsonItem.value()["file"].get<std::string>();
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

    objectModel->cols = 1;
    auto colsIter = objectJson["data"].find("cols");
    if (colsIter != objectJson["data"].end())
    {
        objectModel->cols = colsIter->get<int>();
    }

    objectModel->span = objectModel->cols;
    auto spanIter = objectJson["data"].find("span");
    if (spanIter != objectJson["data"].end())
    {
        objectModel->span = spanIter->get<int>();
    }
}

std::shared_ptr<PlanetModel> loadJsonPlanet(const json& planetJson)
{
    auto planetModel = std::make_shared<PlanetModel>();

    loadObjectModel(planetJson, planetModel);

    auto sunIt = planetJson["data"].find("isSun");
    auto blackHoleIt = planetJson["data"].find("isBlackHole");
    auto storageIt = planetJson["data"].find("isStorage");
    auto hasFoodIt = planetJson["data"].find("hasFood");

    bool isSun = sunIt != planetJson["data"].end() && sunIt->get<bool>();
    bool isBlackHole = blackHoleIt != planetJson["data"].end() && blackHoleIt->get<bool>();
    bool isStorage = storageIt != planetJson["data"].end() && storageIt->get<bool>();
    bool hasFood = hasFoodIt != planetJson["data"].end() && hasFoodIt->get<bool>();

    if (isSun) planetModel->type = PlanetType::SUN;
    else if (isBlackHole) planetModel->type = PlanetType::BLACKHOLE;
    else if (isStorage) planetModel->type = PlanetType::STORAGE;
    else planetModel->type = PlanetType::NORMAL;

    return planetModel;
}

std::shared_ptr<FoodModel> loadJsonFood(const json& foodJson)
{
    auto foodModel = std::make_shared<FoodModel>();

    loadObjectModel(foodJson, foodModel);

    foodModel->cookable = foodJson["data"]["cookable"].get<bool>();

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

    return levelModel;
}
