#include "assetman.h"
#include "loadjson.h"
#include "util.h"
#include "visualizer.h"
#include "recipeeditor.h"
#include "global.h"
#include "savejson.h"

#include "imgui.h"
#include "imfilebrowser.h"

namespace fs = std::filesystem;

static ImGui::FileBrowser s_fileDialog;

const static ImVec4 FAKE_HEADER_COLOR(0.4f, 0.4f, 1.0f, 1.0f);

static void openLevelJson(const std::string& jsonFilename)
{
    g_jsonFilename = jsonFilename;
    g_level = loadJsonLevel(jsonFilename);

    // Initially center on the start planet
    // Or else the initial position is (0, 0) I guess
    for (auto& planet : g_level->planets)
    {
        if (planet->order == PlanetOrder::START)
        {
            g_viz.setWorldPos(planet->pos);
            g_selectedObj = planet;
        }
    }
}

static std::string getFileSelection(const std::string& button, const std::string& windowTitle, const std::filesystem::path& path)
{
    // Hackily use the button text as the ID of the file type being selected
    static std::string id;

    if (ImGui::Button(button.c_str()))
    {
        s_fileDialog.SetTitle(windowTitle);
        s_fileDialog.SetPwd(path);
        s_fileDialog.Open();
        id = button;
    }

    s_fileDialog.Display();

    if (s_fileDialog.HasSelected() && id == button)
    {
        std::string ret = s_fileDialog.GetSelected().string();
        s_fileDialog.ClearSelected();
        return ret;
    }

    return "";
}

static void showJsonFileState()
{
    if (g_level)
    {
        std::string file = std::filesystem::path(g_jsonFilename).filename().string();
        ImGui::TextColored(FAKE_HEADER_COLOR, "JSON loaded: %s", file.c_str());
    } else
    {
        ImGui::TextColored(FAKE_HEADER_COLOR, "No JSON file loaded");
    }

    static std::string openErrorMsg;

    std::filesystem::path defaultJsonPath = g_assetMan.getAssetPathRoot() / "json";
    std::string levelJsonPath = getFileSelection("Open", "Open JSON Level", defaultJsonPath);
    if (levelJsonPath.size() != 0)
    {
        try
        {
            openLevelJson(levelJsonPath);
        } catch (const std::exception& ex)
        {
            openErrorMsg = ex.what();
            ImGui::OpenPopup("Cannot open level");
        }
    }

    if (ImGui::BeginPopupModal("Cannot open level"))
    {
        ImGui::Text("Cannot open level: %s", openErrorMsg.c_str());
        if (ImGui::Button("Ok")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    static std::string saveErrorMsg;

    if (g_level)
    {
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            try
            {
                saveJsonLevel(g_jsonFilename, g_level);
            } catch (const std::exception& ex)
            {
                ImGui::OpenPopup("Cannot save level");
                saveErrorMsg = ex.what();
            }
        }

        if (ImGui::BeginPopupModal("Cannot save level"))
        {
            ImGui::Text("Cannot save level: %s", saveErrorMsg.c_str());
            if (ImGui::Button("Ok")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }
}

static bool showRedButton(const std::string& label)
{
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0/7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0/7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0/7.0f, 0.8f, 0.8f));
    bool ret = ImGui::Button(label.c_str());
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    return ret;
}

static void showLevelProperties()
{
    ImGui::TextColored(FAKE_HEADER_COLOR, "Level Properties");

    if (!g_level)
    {
        ImGui::Text("No JSON level loaded");
        return;
    }

    ImGui::InputInt("Level number", &g_level->levelNumber);
    ImGui::InputFloat("Level timer", &g_level->levelTimer);

    fs::path texDirPath = g_assetMan.getAssetPathRoot() / "textures";

    // Add planet button
    std::filesystem::path texPath = getFileSelection("Add planet", "Select planet texture", texDirPath);
    if (!texPath.empty())
    {
        auto planet = std::make_shared<PlanetModel>();
        planet->tex = g_assetMan.loadTexture(texPath);
        planet->pos = g_viz.getWorldPos();
        planet->anchor = ImVec2(0.5, 0.5);
        planet->scale = 0.5;
        planet->cols = 2;
        planet->span = 2;
        planet->type = PlanetType::NORMAL;
        planet->hasFood = false;
        planet->order = PlanetOrder::MIDDLE;

        g_level->planets.emplace_back(planet);
        g_selectedObj = planet;
    }
    ImGui::SameLine();

    // Add food button
    texPath = getFileSelection("Add food", "Select food texture", texDirPath);
    if (!texPath.empty())
    {
        auto food = std::make_shared<FoodModel>();
        food->tex = g_assetMan.loadTexture(texPath);
        food->pos = g_viz.getWorldPos();
        food->anchor = ImVec2(0.5, 0.5);
        food->scale = 0.5;
        food->cols = 1;
        food->span = 1;
        food->cookable = false;
        food->seasonable = false;

        g_level->foods.emplace_back(food);
        g_selectedObj = food;
    }
    ImGui::SameLine();

    // Add player button
    texPath = getFileSelection("Add player", "Select player texture", texDirPath);
    if (!texPath.empty())
    {
        auto player = std::make_shared<ObjectModel>();
        player->tex = g_assetMan.loadTexture(texPath);
        player->pos = g_viz.getWorldPos();
        player->anchor = ImVec2(0.5, 0.5);
        player->scale = 0.5;
        player->cols = 1;
        player->span = 1;

        g_level->player = player;
        g_selectedObj = player;
    }
    ImGui::SameLine();

    texPath = getFileSelection("Add customer", "Select player texture", texDirPath);
    if (!texPath.empty())
    {
        auto customer = std::make_shared<ObjectModel>();
        customer->tex = g_assetMan.loadTexture(texPath);
        customer->pos = g_viz.getWorldPos();
        customer->anchor = ImVec2(0.5, 0.5);
        customer->scale = 0.5;
        customer->cols = 1;
        customer->span = 1;

        g_level->customer = customer;
        g_selectedObj = customer;
    }
}

static void showPropertiesEditor()
{
    ImGui::Begin("Properties Editor");

    showJsonFileState();
    ImGui::Separator();

    showLevelProperties();
    ImGui::Separator();

    ImGui::TextColored(FAKE_HEADER_COLOR, "Object Properties");
    if (!g_selectedObj)
    {
        ImGui::Text("No selected object");
        ImGui::End();
        return;
    }

    // Is casting like this bad?
    ImGui::InputFloat2("Position", reinterpret_cast<float *>(&g_selectedObj->pos), "%.3f");
    ImGui::InputFloat2("Anchor", reinterpret_cast<float *>(&g_selectedObj->anchor), "%.3f");
    ImGui::SliderFloat("Scale", &g_selectedObj->scale, 0.1, 2.0);

    ImGui::InputInt("Texture columns", &g_selectedObj->cols);
    ImGui::InputInt("Texture span", &g_selectedObj->span);

    // Handle object deletion
    if (g_selectedObj && showRedButton("Delete object"))
    {
        if (g_selectedObj == g_level->player) g_level->player = nullptr;
        else if (g_selectedObj == g_level->customer) g_level->customer = nullptr;
        else
        {
            auto& planets = g_level->planets;
            auto& foods = g_level->foods;
            planets.erase(std::remove(planets.begin(), planets.end(), g_selectedObj), planets.end());
            foods.erase(std::remove(foods.begin(), foods.end(), g_selectedObj), foods.end());
        }

        g_selectedObj = nullptr;
    }

    ImGui::Separator();

    // Show planet-specific properties if this is a planet
    auto selectedPlanet = std::dynamic_pointer_cast<PlanetModel>(g_selectedObj);
    if (selectedPlanet)
    {
        ImGui::TextColored(FAKE_HEADER_COLOR, "Planet Properties");

        int order = static_cast<int>(selectedPlanet->order);
        ImGui::RadioButton("Start planet", &order, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Middle planet", &order, 1);
        ImGui::SameLine();
        ImGui::RadioButton("End planet", &order, 2);
        selectedPlanet->order = static_cast<PlanetOrder>(order);

        int type = static_cast<int>(selectedPlanet->type);
        ImGui::RadioButton("Normal", &type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Sun", &type, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Blackhole", &type, 2);
        ImGui::SameLine();
        ImGui::RadioButton("Storage", &type, 3);
        ImGui::SameLine();
        ImGui::RadioButton("Season", &type, 4);
        selectedPlanet->type = static_cast<PlanetType>(type);

        ImGui::Checkbox("Has food", &selectedPlanet->hasFood);
    }

    // Show food-specific properties if this is a food
    auto selectedFood = std::dynamic_pointer_cast<FoodModel>(g_selectedObj);
    if (selectedFood)
    {
        ImGui::TextColored(FAKE_HEADER_COLOR, "Food Properties");
        ImGui::Checkbox("Cookable", &selectedFood->cookable);
        ImGui::Checkbox("Seasonable", &selectedFood->seasonable);
    }

    if (g_selectedObj == g_level->player)
    {
        ImGui::TextColored(FAKE_HEADER_COLOR, "Player properties");
    }

    if (g_selectedObj == g_level->customer)
    {
        ImGui::TextColored(FAKE_HEADER_COLOR, "Customer properties");
    }

    ImGui::End();
}

void initEditor()
{
    g_assetMan.init();
    g_gravRangeTex = g_assetMan.loadTexture(g_assetMan.getAssetPathRoot() / "textures" / "range.png", "range");
    g_showGravRanges = true;
    s_fileDialog.SetTitle("Select file");
}

void runEditor()
{
//    ImGui::ShowDemoWindow();
    showLevelVisualizer();
    showPropertiesEditor();
    showRecipeEditor();
}
