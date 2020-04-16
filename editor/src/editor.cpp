#include "assetman.h"
#include "loadjson.h"
#include "util.h"
#include "visualizer.h"
#include "global.h"
#include "savejson.h"

#include "imgui.h"
#include "imfilebrowser.h"

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

    std::string levelJsonPath = getFileSelection("Open", "Open JSON Level", "../assets/json");
    if (levelJsonPath.size() != 0)
    {
        openLevelJson(levelJsonPath);
    }

    if (g_level)
    {
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            saveJsonLevel(g_jsonFilename, g_level);
        }
    }
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

    std::filesystem::path texDirPath = std::filesystem::current_path().parent_path() / "assets" / "textures";
    texDirPath = texDirPath.lexically_normal();

    // Add planet button
    std::string texPath = getFileSelection("Add planet", "Select planet texture", texDirPath);
    if (!texPath.empty())
    {
        auto planet = std::make_shared<PlanetModel>();
        planet->tex = g_assetMan.textureFromAbsPath(texPath);
        planet->pos = g_viz.getWorldPos();
        planet->anchor = ImVec2(0.5, 0.5);
        planet->scale = 0.5;
        planet->cols = 2;
        planet->span = 2;
        planet->isSun = false;
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
        food->tex = g_assetMan.textureFromAbsPath(texPath);
        food->pos = g_viz.getWorldPos();
        food->anchor = ImVec2(0.5, 0.5);
        food->scale = 0.5;
        food->cols = 1;
        food->span = 1;
        food->cookable = false;

        g_level->foods.emplace_back(food);
        g_selectedObj = food;
    }
    ImGui::SameLine();

    // Add player button
    texPath = getFileSelection("Add player", "Select player texture", texDirPath);
    if (!texPath.empty())
    {
        auto player = std::make_shared<ObjectModel>();
        player->tex = g_assetMan.textureFromAbsPath(texPath);
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
        customer->tex = g_assetMan.textureFromAbsPath(texPath);
        customer->pos = g_viz.getWorldPos();
        customer->anchor = ImVec2(0.5, 0.5);
        customer->scale = 0.5;
        customer->cols = 1;
        customer->span = 1;

        g_level->customer = customer;
        g_selectedObj = customer;
    }

    ImGui::Checkbox("Show gravity ranges", &g_showGravRanges);
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

    ImGui::Text("Texture filepath: %s", g_assetMan.assetPathFromTexture(g_selectedObj->tex).c_str());

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
        ImGui::TextColored(FAKE_HEADER_COLOR, "Planet Properties");

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
        ImGui::TextColored(FAKE_HEADER_COLOR, "Food Properties");
        ImGui::Checkbox("Cookable", &selectedFood->cookable);
    }

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
    g_gravRangeTex = g_assetMan.textureFromAssetPath("textures/range.png");
    g_showGravRanges = true;
    s_fileDialog.SetTitle("Select file");
}

void runEditor()
{
    ImGui::ShowDemoWindow();
    showLevelVisualization();
    showPropertiesEditor();
}
