#include "savejson.h"
#include "util.h"
#include "global.h"

#include "json.hpp"

#include <unordered_map>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

// Map of texture to short name
typedef std::unordered_map<std::shared_ptr<Texture>, std::string> TexTable;

static json genVecJson(ImVec2 vec)
{
    return json::array({vec.x, vec.y});
}

static json genObjectJson(const std::shared_ptr<ObjectModel>& obj, const TexTable& texTable)
{
    return {
        {"type", "Animation"},
        {"data", {
                 {"texture", texTable.at(obj->tex)},
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

static json genPlanetJson(const std::shared_ptr<PlanetModel>& planet, const TexTable& texTable)
{
    json planetJson = genObjectJson(planet, texTable);
    planetJson["data"]["hasFood"] = planet->hasFood;
    planetJson["data"]["isSun"] = planet->isSun;
    return planetJson;
}

static json genPlanetRingJson(const std::shared_ptr<PlanetModel>& planet, const TexTable& texTable)
{
    json j = genObjectJson(planet, texTable);
    j["data"]["texture"] = "range";
    j["data"]["cols"] = 5;
    j["data"]["frame"] = 0;
    j["data"]["scale"] = j["data"]["scale"].get<float>() * 3;
    return j;
}

static std::pair<json, json> genPlanetsJson(const std::shared_ptr<LevelModel>& level, const TexTable& texTable)
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

    planetsJson["children"]["startPlanet"] = genPlanetJson(startPlanet, texTable);
    planetsJson["children"]["endPlanet"] = genPlanetJson(endPlanet, texTable);
    planetRingsJson["children"]["startPlanetRange"] = genPlanetRingJson(startPlanet, texTable);
    planetRingsJson["children"]["endPlanetRange"] = genPlanetRingJson(endPlanet, texTable);

    int planetIdx = 1;
    for (auto& planet : level->planets)
    {
        if (planet->order == PlanetOrder::MIDDLE)
        {
            std::string planetName = "planet" + std::to_string(planetIdx);
            std::string planetRangeName = "planet" + std::to_string(planetIdx++) + "Range";
            planetsJson["children"][planetName] = genPlanetJson(planet, texTable);
            planetRingsJson["children"][planetRangeName] = genPlanetRingJson(planet, texTable);
        }
    }

    return {planetsJson, planetRingsJson};
}

static json genFoodsJson(const std::shared_ptr<LevelModel>& level, const TexTable& texTable)
{
    json foodListJson;
    foodListJson["type"] = "Node";

    for (size_t foodIdx = 0; foodIdx != level->foods.size(); ++foodIdx)
    {
        auto& food = level->foods[foodIdx];
        json foodJson = genObjectJson(food, texTable);
        foodJson["data"]["cookable"] = food->cookable;

        std::string foodName = "food" + std::to_string(foodIdx + 1);
        foodListJson["children"][foodName] = foodJson;
    }

    return foodListJson;
}

void saveJsonLevel(const std::string &filename, const std::shared_ptr<LevelModel> &level)
{
    // Create initial level json
    json levelJson;

    std::string levelNumStr = "lv" + std::to_string(level->levelNumber);

    // Create texture table
    int texId = 0;
    TexTable texTable;
    auto allObjs = getAllLevelObjects(level);
    for (auto& obj : allObjs)
    {
        auto texTableIt = texTable.find(obj->tex);
        if (texTableIt == texTable.end())
        {
            texTable[obj->tex] = "tex" + std::to_string(texId++);
        }
    }

    // Add texture table to json
    for (auto& item : texTable)
    {
        levelJson["textures"][item.second]["file"] = g_assetMan.assetPathFromTexture(item.first);
    }

    // Add planets and foods
    auto planetJsonPair = genPlanetsJson(level, texTable);
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planets"] = planetJsonPair.first;
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["planetRanges"] = planetJsonPair.second;

    // Add foods
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["food"] = genFoodsJson(level, texTable);

    // Add player and customer
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["customer"] = genObjectJson(level->customer, texTable);
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["player"] = genObjectJson(level->player, texTable);

    // Add food and planet counts
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numPlanets"]["type"] = "Node";
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numPlanets"]["data"]["num"] = level->planets.size() - 2;
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numFood"]["type"] = "Node";
    levelJson["scenes"][levelNumStr]["children"]["game"]["children"]["numFood"]["data"]["num"] = level->foods.size();

    // Add fonts (unnecessary?)
    levelJson["fonts"]["felt32"]["file"] = "fonts/MarkerFelt.ttf";
    levelJson["fonts"]["felt32"]["size"] = 32;

    // Add some textures manually
    levelJson["textures"]["range"]["file"] = "textures/range.png";
    levelJson["textures"]["space"]["file"] = "textures/space.png";
    levelJson["textures"]["space"]["wrapS"] = "repeat";
    levelJson["textures"]["space"]["wrapT"] = "repeat";
    levelJson["scenes"][levelNumStr]["children"]["background"] = R"(
     {
        "type": "Image",
        "data": {
            "texture":  "space",
            "position": [0, 0],
            "scale":    0.5,
            "polygon":  [0,0,0,2540,2540,2540,4500,2540,4500,0,0,0],
            "anchor":   [0.5,0.5]
        }
    }
    )"_json;

    // Save json to file
    std::ofstream ofile(filename);
    ofile << std::setw(4) << levelJson << std::endl;
}
