#include "loadjson.h"
#include "global.h"

#include "json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

typedef std::unordered_map<std::string, std::shared_ptr<Texture>> TexTable;

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

void loadObjectModel(const json& objectJson, const TexTable& texTable, std::shared_ptr<ObjectModel> objectModel)
{
    std::string texName = objectJson["data"]["texture"].get<std::string>();
    objectModel->tex = texTable.at(texName);
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

std::shared_ptr<PlanetModel> loadJsonPlanet(const json& planetJson, const TexTable& texTable)
{
    auto planetModel = std::make_shared<PlanetModel>();

    loadObjectModel(planetJson, texTable, planetModel);

    planetModel->hasFood = planetJson["data"]["hasFood"].get<bool>();
    planetModel->isSun = planetJson["data"]["isSun"].get<bool>();

    return planetModel;
}

std::shared_ptr<FoodModel> loadJsonFood(const json& foodJson, const TexTable& texTable)
{
    auto foodModel = std::make_shared<FoodModel>();

    loadObjectModel(foodJson, texTable, foodModel);

    foodModel->cookable = foodJson["data"]["cookable"].get<bool>();

    return foodModel;
}

std::shared_ptr<LevelModel> loadJsonLevel(const std::string& filename)
{
    std::ifstream f(filename);
    json levelJson;
    f >> levelJson;

    auto levelModel = std::make_shared<LevelModel>();

    // Get level number
    std::string levelNumStr = levelJson["scenes"].items().begin().key();
    levelModel->levelNumber = std::stoi(levelNumStr.substr(2, levelNumStr.size()));

    // Build a little list of textures
    TexTable texTable;
    for (auto& texJson : levelJson["textures"].items())
    {
        std::string shortName = texJson.key();
        std::string fileName = texJson.value()["file"].get<std::string>();

        auto tex = g_assetMan.textureFromAssetPath(fileName);
        assert(tex != nullptr);
        texTable.emplace(shortName, tex);
    }

    // Load planets
    auto& planetMapJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planets"]["children"];
    for (auto& planetJsonItem : planetMapJson.items())
    {
        auto planet = loadJsonPlanet(planetJsonItem.value(), texTable);
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
        levelModel->foods.emplace_back(loadJsonFood(foodJsonItem.value(), texTable));
    }

    // Load player
    auto& playerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["player"];
    levelModel->player = std::make_shared<ObjectModel>();
    loadObjectModel(playerJson, texTable, levelModel->player);

    // Load customer
    auto& customerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["customer"];
    levelModel->customer = std::make_shared<ObjectModel>();
    loadObjectModel(customerJson, texTable, levelModel->customer);

    return levelModel;
}
