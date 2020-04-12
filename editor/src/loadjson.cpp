#include <fstream>
#include <iostream>
#include "loadjson.h"

#include "json.hpp"

using json = nlohmann::json;

typedef std::unordered_map<std::string, Texture> TexTable;

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

PlanetModel loadJsonPlanet(const json& planetJson, const TexTable& texTable)
{
    PlanetModel planetModel;

    std::string texName = planetJson["data"]["texture"].get<std::string>();
    planetModel.tex = texTable.at(texName);
    planetModel.pos = loadJsonCoord(planetJson["data"]["position"]);
    planetModel.anchor = loadJsonCoord(planetJson["data"]["anchor"]);
    planetModel.scale = loadJsonCoord(planetJson["data"]["scale"]).x;

    planetModel.hasFood = planetJson["data"]["hasFood"].get<bool>();
    planetModel.isSun = planetJson["data"]["isSun"].get<bool>();

    return planetModel;
}

FoodModel loadJsonFood(const json& foodJson, const TexTable& texTable)
{
    FoodModel foodModel;

    std::string texName = foodJson["data"]["texture"].get<std::string>();
    foodModel.tex = texTable.at(texName);
    foodModel.pos = loadJsonCoord(foodJson["data"]["position"]);
    foodModel.anchor = loadJsonCoord(foodJson["data"]["anchor"]);
    foodModel.scale = loadJsonCoord(foodJson["data"]["scale"]).x;

    foodModel.cols = foodJson["data"]["cols"].get<int>();
    foodModel.cookable = foodJson["data"]["cookable"].get<bool>();

    return foodModel;
}

ObjectModel loadObjectModel(const json& objectJson, const TexTable& texTable)
{
    ObjectModel objectModel = {};

    std::string texName = objectJson["data"]["texture"].get<std::string>();
    objectModel.tex = texTable.at(texName);
    objectModel.pos = loadJsonCoord(objectJson["data"]["position"]);
    objectModel.anchor = loadJsonCoord(objectJson["data"]["anchor"]);
    objectModel.scale = loadJsonCoord(objectJson["data"]["scale"]).x;

    return objectModel;
}

LevelModel loadJsonLevel(std::string filename)
{
    std::ifstream f(filename);
    json levelJson;
    f >> levelJson;

    LevelModel levelModel = {};

    // Get level number
    std::string levelNumStr = levelJson["scenes"].items().begin().key();
    levelModel.levelNumber = std::stoi(levelNumStr.substr(2, levelNumStr.size()));

    // Build a little list of textures
    TexTable texTable;
    for (auto& texJson : levelJson["textures"].items())
    {
        std::string shortName = texJson.key();
        std::string fileName = texJson.value()["file"].get<std::string>();

        Texture tex{};
        loadTextureFromFile(fileName.c_str(), tex);
        texTable.emplace(shortName, tex);
    }

    // Load planets
    auto& planetMapJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planets"]["children"];
    for (auto& planetJsonItem : planetMapJson.items())
    {
        PlanetModel planet = loadJsonPlanet(planetJsonItem.value(), texTable);
        if (planetJsonItem.key() == "startPlanet")
        {
            levelModel.startPlanet = planet;
        }
        else if (planetJsonItem.key() == "endPlanet")
        {
            levelModel.endPlanet = planet;
        }
        else
        {
            levelModel.planets.emplace_back(planet);
        }
    }

    // Load foods
    auto& foodMapJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["food"]["children"];
    for (auto& foodJsonItem : foodMapJson.items())
    {
        levelModel.foods.emplace_back(loadJsonFood(foodJsonItem.value(), texTable));
    }

    // Load player
    auto& playerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["player"];
    levelModel.player = loadObjectModel(playerJson, texTable);

    // Load customer
    auto& customerJson = levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["customer"];
    levelModel.customer = loadObjectModel(customerJson, texTable);

    return levelModel;
}
