#include "editor.h"
#include "textures.h"
#include "loadjson.h"
#include "util.h"
#include "visualizer.h"
#include "global.h"

#include "imgui.h"
#include "savejson.h"

static void showPropertiesEditor()
{
    ImVec4 fakeHeaderColor(0.4f, 0.4f, 1.0f, 1.0f);

    ImGui::Begin("Properties Editor");

    ImGui::TextColored(fakeHeaderColor, "Level Properties");

    ImGui::InputInt("Level number", &g_level->levelNumber);

    ImGui::Button("Add planet");
    ImGui::SameLine();
    ImGui::Button("Add food");

    // TODO remove
    if (ImGui::Button("Export JSON"))
    {
        saveJsonLevel("poop.json", g_level);
    }

    ImGui::Checkbox("Show gravity ranges", &g_showGravRanges);

    ImGui::Separator();

    if (!g_selectedObj)
    {
        ImGui::Text("No selected object");
        ImGui::End();
        return;
    }

    ImGui::TextColored(fakeHeaderColor, "Object Properties");

    ImGui::Text("Texture filepath: %s", g_selectedObj->tex.filename.c_str());

    // Is casting like this bad?
    ImGui::InputFloat2("Position", reinterpret_cast<float *>(&g_selectedObj->pos), "%.3f");
    ImGui::InputFloat2("Anchor", reinterpret_cast<float *>(&g_selectedObj->anchor), "%.3f");
    ImGui::SliderFloat("Scale", &g_selectedObj->scale, 0.1, 2.0);

    ImGui::InputInt("Texture columns", &g_selectedObj->cols);
    ImGui::InputInt("Texture span", &g_selectedObj->span);

    // Dummies for now
    bool isCustomer = g_selectedObj == g_level->customer;
    bool isPlayer = g_selectedObj == g_level->player;
    ImGui::Checkbox("Is player", &isPlayer);
    ImGui::SameLine();
    ImGui::Checkbox("Is customer", &isCustomer);

    ImGui::Separator();

    // Show planet-specific properties if this is a planet
    auto selectedPlanet = std::dynamic_pointer_cast<PlanetModel>(g_selectedObj);
    if (selectedPlanet)
    {
        ImGui::TextColored(fakeHeaderColor, "Planet Properties");

        int order = static_cast<int>(selectedPlanet->order);
        ImGui::RadioButton("Start planet", &order, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Middle planet", &order, 1);
        ImGui::SameLine();
        ImGui::RadioButton("End planet", &order, 2);
        selectedPlanet->order = static_cast<PlanetOrder>(order);

        ImGui::Checkbox("Sun", &selectedPlanet->isSun);
        ImGui::Checkbox("Has food", &selectedPlanet->hasFood);
    }


    // Show food-specific properties if this is a food
    auto selectedFood = std::dynamic_pointer_cast<FoodModel>(g_selectedObj);
    if (selectedFood)
    {
        ImGui::TextColored(fakeHeaderColor, "Food Properties");
        ImGui::Checkbox("Cookable", &selectedFood->cookable);
    }

    ImGui::End();
}

static void showHelp()
{
    ImGui::Begin("Help");

    ImGui::Text("LEVEL VISUALIZATION:");
    ImGui::BulletText("Click a planet, food, or other object to select it");
    ImGui::BulletText("Click and drag a planet, food, or other object to change its position in the level");
    ImGui::BulletText("Click and drag in empty space to pan the level");
    ImGui::Separator();

    ImGui::End();
}

//static void showDemoStuff()
//{
//    // Our state
//    static bool show_demo_window = true;
//    static bool show_another_window = false;
//
//    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);
//
//    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
//    {
//        static float f = 0.0f;
//        static int counter = 0;
//
//        ImGui::Begin(
//                "Hello, world!");                          // Create a window called "Hello, world!" and append into it.
//
//        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
//        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//        ImGui::Checkbox("Another Window", &show_another_window);
//
//        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
////        ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color
//
//        if (ImGui::Button(
//                "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//            counter++;
//        ImGui::SameLine();
//        ImGui::Text("counter = %d", counter);
//
//        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
//                    ImGui::GetIO().Framerate);
//        ImGui::End();
//    }
//
//    // 3. Show another simple window.
//    if (show_another_window) {
//        ImGui::Begin("Another Window",
//                     &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
//        ImGui::Text("Hello from another window!");
//        if (ImGui::Button("Close Me"))
//            show_another_window = false;
//        ImGui::End();
//    }
//}

void initEditor()
{
    g_level = loadJsonLevel("examples/level1.json");

    // Initially center on the start planet
    // Or else the initial position is (0, 0) I guess
    for (auto& planet : g_level->planets)
    {
        if (planet->order == PlanetOrder::START)
        {
            viz.setWorldPos(planet->pos);
            g_selectedObj = planet;
        }
    }

    IM_ASSERT(loadTextureFromFile("../assets/textures/range.png", g_gravRangeTex));
    g_showGravRanges = true;
}


void runEditor()
{
//    showDemoStuff();
//    showHelp();
    showLevelVisualization();
    showPropertiesEditor();
}
