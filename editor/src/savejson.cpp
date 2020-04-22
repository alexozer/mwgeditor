#include "savejson.h"
#include "util.h"
#include "global.h"

#include "json.hpp"

#include <unordered_map>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;
namespace fs = std::filesystem;

void validateLevelForExport()
{
    if (!g_level->player) throw std::runtime_error("Level must contain a player");
    if (!g_level->customer) throw std::runtime_error("Level must contain a customer");

    int startPlanets = 0;
    int endPlanets = 0;
    for (auto& planet : g_level->planets)
    {
        if (planet->order == PlanetOrder::START) startPlanets++;
        if (planet->order == PlanetOrder::END) endPlanets++;
    }

    if (startPlanets != 1) throw std::runtime_error("Level must contain a single starting planet.\nThere are currently: " + std::to_string(startPlanets));
    if (endPlanets != 1) throw std::runtime_error("Level must contain a single ending planet.\nThere are currently: " + std::to_string(endPlanets));
}

static json genVecJson(ImVec2 vec)
{
    return json::array({vec.x, vec.y});
}

static json genObjectJson(const std::shared_ptr<ObjectModel>& obj)
{
    return {
        {"type", "Animation"},
        {"data", {
                 {"texture", obj->tex->shortName},
                 {"cols", obj->cols},
                 {"span", obj->span},
                 {"frame", 0},
                 {"scale", obj->scale},
                 {"position", genVecJson(obj->pos)},
                 {"anchor", genVecJson(obj->anchor)},
             }
        }
    };
}

static json genPlanetJson(const std::shared_ptr<PlanetModel>& planet)
{
    json planetJson = genObjectJson(planet);
    planetJson["data"]["hasFood"] = planet->hasFood;
    planetJson["data"]["isSun"] = planet->type == PlanetType::SUN;
    planetJson["data"]["isStorage"] = planet->type == PlanetType::STORAGE;
    planetJson["data"]["isBlackHole"] = planet->type == PlanetType::BLACKHOLE;
    return planetJson;
}

static json genPlanetRingJson(const std::shared_ptr<PlanetModel>& planet)
{
    json j = genObjectJson(planet);
    j["data"]["texture"] = "range";
    j["data"]["cols"] = 5;
    j["data"]["span"] = 5;
    j["data"]["frame"] = 0;
    j["data"]["scale"] = j["data"]["scale"].get<float>() * 3;
    return j;
}

static std::pair<json, json> genPlanetsJson(const std::shared_ptr<LevelModel>& level)
{
    // Find start planet and end planet (there should be at least 1 guaranteed)
    auto startIt = std::find_if(
            level->planets.begin(),
            level->planets.end(),
            [](std::shared_ptr<PlanetModel> planet) {
                return planet->order == PlanetOrder::START;
            });
    IM_ASSERT(startIt != level->planets.end());
    std::shared_ptr<PlanetModel> startPlanet = *startIt;

    auto endIt = std::find_if(
            level->planets.begin(),
            level->planets.end(),
            [](std::shared_ptr<PlanetModel> planet) {
                return planet->order == PlanetOrder::END;
            });
    IM_ASSERT(endIt != level->planets.end());
    std::shared_ptr<PlanetModel> endPlanet = *endIt;

    // Planets json

    json planetsJson, planetRingsJson;
    planetsJson["type"] = "Node";
    planetRingsJson["type"] = "Node";

    planetsJson["children"]["startPlanet"] = genPlanetJson(startPlanet);
    planetsJson["children"]["endPlanet"] = genPlanetJson(endPlanet);
    planetRingsJson["children"]["startPlanetRange"] = genPlanetRingJson(startPlanet);
    planetRingsJson["children"]["endPlanetRange"] = genPlanetRingJson(endPlanet);

    int planetIdx = 1;
    for (auto& planet : level->planets)
    {
        if (planet->order == PlanetOrder::MIDDLE)
        {
            std::string planetName = "planet" + std::to_string(planetIdx);
            std::string planetRangeName = "planet" + std::to_string(planetIdx++) + "Range";
            planetsJson["children"][planetName] = genPlanetJson(planet);
            planetRingsJson["children"][planetRangeName] = genPlanetRingJson(planet);
        }
    }

    return {planetsJson, planetRingsJson};
}

static json genFoodsJson(const std::shared_ptr<LevelModel>& level)
{
    json foodListJson;
    foodListJson["type"] = "Node";

    for (size_t foodIdx = 0; foodIdx != level->foods.size(); ++foodIdx)
    {
        auto& food = level->foods[foodIdx];
        json foodJson = genObjectJson(food);
        foodJson["data"]["cookable"] = food->cookable;

        std::string foodName = "food" + std::to_string(foodIdx + 1);
        foodListJson["children"][foodName] = foodJson;
    }

    return foodListJson;
}

void saveAssetsJson()
{
    fs::path assetsJsonPath = AssetMan::getAssetPathRoot() / "json" / "assets_demo.json";
    std::ifstream fin(assetsJsonPath);
    json assetsJson;
    fin >> assetsJson;
    auto& texturesJson = assetsJson["textures"];

    for (auto& tex : g_assetMan.getTextures())
    {
        texturesJson[tex->shortName]["file"] = AssetMan::getAssetPathStr(tex->filePath);
    }

    std::ofstream fout(assetsJsonPath);
    fout << std::setw(4) << assetsJson << std::endl;
}

void saveJsonLevel(const std::string &filename, const std::shared_ptr<LevelModel> &level)
{
    validateLevelForExport();

    // Create initial level json
    json levelJson;

    std::string levelNumStr = "lv" + std::to_string(level->levelNumber);

    saveAssetsJson();

    // Add planets and foods
    auto planetJsonPair = genPlanetsJson(level);
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planets"] = planetJsonPair.first;
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planetRanges"] = planetJsonPair.second;

    // Add foods
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["food"] = genFoodsJson(level);

    // Add player and customer
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["customer"] = genObjectJson(level->customer);
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["player"] = genObjectJson(level->player);

    // Add food and planet counts
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numPlanets"]["type"] = "Node";
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numPlanets"]["data"]["num"] = level->planets.size() - 2;
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numFood"]["type"] = "Node";
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numFood"]["data"]["num"] = level->foods.size();

    levelJson["scenes"][levelNumStr]["type"] = "Node";
    levelJson["scenes"][levelNumStr]["children"]["game"]["type"] = "Node";

    levelJson["scenes"][levelNumStr]["children"]["background"] = R"(
     {
        "type": "Image",
        "data": {
            "texture":  "space",
            "position": [0, 0],
            "polygon":  [0,0,0,2540,2540,2540,4500,2540,4500,0,0,0],
            "anchor":   [0.5,0.5]
        }
    }
    )"_json;

    // Save json to file
    std::ofstream ofile(filename);
    ofile << std::setw(4) << levelJson << std::endl;
}
