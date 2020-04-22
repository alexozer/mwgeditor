#include "recipeeditor.h"
#include "global.h"

#include "imgui.h"

#include <vector>

void showRecipeEditor()
{
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Recipe Editor");
    if (!g_level)
    {
        ImGui::End();
        return;
    }

    std::vector<std::string> foodNames;
    std::vector<const char *> foodNameCstrs;

    for (auto& food : g_level->foods)
    {
        std::string basicName = food->tex->filePath.filename().u8string();
        foodNames.push_back(basicName);
    }
    for (auto& name : foodNames)
    {
        foodNameCstrs.push_back(name.c_str());
    }

    static int idx = 0;

    // Make currently selected food the current list item, if a food is selected
    auto foodIt = std::find(g_level->foods.begin(), g_level->foods.end(), g_selectedObj);
    if (foodIt != g_level->foods.end())
    {
        idx = foodIt - g_level->foods.begin();
    }

    // Check if index is valid, in case we deleted a food
    if (idx >= g_level->foods.size()) idx = g_level->foods.size() - 1;

    int oldIdx = idx;
    ImGui::ListBox("Recipe order", &idx, foodNameCstrs.data(), foodNameCstrs.size(), 10);
    if (idx != oldIdx)
    {
        g_selectedObj = g_level->foods[idx];
        g_viz.setWorldPos(g_selectedObj->pos);
    }

    int desIdx = idx;

    // Support swapping foods
    if (ImGui::Button("Move down")) desIdx = idx + 1;
    ImGui::SameLine();
    if (ImGui::Button("Move up")) desIdx = idx - 1;

    if (desIdx < 0) desIdx = 0;
    else if (desIdx >= g_level->foods.size()) desIdx = g_level->foods.size() - 1;

    std::swap(g_level->foods[idx], g_level->foods[desIdx]);

    ImGui::End();
}

