#include "savejson.h"
#include "util.h"

#include "json.hpp"

#include <unordered_map>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

// Map of texture filename to "short name"
typedef std::unordered_map<std::string, std::string> TexTable;

json genVecJson(ImVec2 vec)
{
    return json::array({vec.x, vec.y});
}

json genObjectJson(const std::shared_ptr<ObjectModel>& obj, const TexTable& texTable)
{
    return {
        {"type", "Animation"},
        {"data", {
                 {"texture", texTable.at(obj->tex.filename)},
                 {"cols", obj->cols},
                 {"frame", 0},
                 {"scale", obj->scale},
                 {"position", genVecJson(obj->pos)},
                 {"anchor", genVecJson(obj->anchor)},
             }
        }
    };
}

json genPlanetJson(const std::shared_ptr<PlanetModel>& planet, const TexTable& texTable)
{
    json planetJson = genObjectJson(planet, texTable);
    planetJson["data"]["hasFood"] = planet->hasFood;
    planetJson["data"]["isSun"] = planet->isSun;
    return planetJson;
}

json genPlanetsJson(const std::shared_ptr<LevelModel>& level, const TexTable& texTable)
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

    // Add planets to json

    json planetsJson;
    planetsJson["type"] = "Node";
    planetsJson["children"]["startPlanet"] = genPlanetJson(startPlanet, texTable);
    planetsJson["children"]["endPlanet"] = genPlanetJson(endPlanet, texTable);

    int planetIdx = 1;
    for (auto& planet : level->planets)
    {
        if (planet->order == PlanetOrder::MIDDLE)
        {
            std::string planetName = "planet" + std::to_string(planetIdx++);
            planetsJson["children"][planetName] = genPlanetJson(planet, texTable);
        }
    }

    return planetsJson;
}

void saveJsonLevel(const std::string &filename, const std::shared_ptr<LevelModel> &level)
{
    // Create initial level json
    json levelJson;

    // Create texture table
    int texId = 0;
    TexTable texTable;
    auto allObjs = getAllLevelObjects(level);
    for (auto& obj : allObjs)
    {
        std::string texPath = obj->tex.filename;
        auto texTableIt = texTable.find(texPath);
        if (texTableIt == texTable.end())
        {
            texTable[texPath] = "tex" + std::to_string(texId++);
        }
    }

    // Add texture table to json
    for (auto& item : texTable)
    {
        std::string prefix = "../assets/";
        std::string fixedPath(item.first.substr(prefix.size(), item.first.size()));
        levelJson["textures"][item.second]["file"] = fixedPath;
    }

    // TODO use real level number
    levelJson["scenes"]["lv1"]["children"]["game"]["children"]["planets"] = genPlanetsJson(level, texTable);

    // Save json to file
    std::ofstream ofile(filename);
    ofile << std::setw(4) << levelJson << std::endl;
}
