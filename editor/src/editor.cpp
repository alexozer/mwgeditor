#include "editor.h"
#include "textures.h"
#include "loadjson.h"

#include "imgui.h"

struct VisualizationModel
{
    float zoom;
    ImVec2 pos;
};

static LevelModel level = {};
static VisualizationModel viz = {};

static ImVec2 vizToWorldSpace(ImVec2 pos)
{
    return ImVec2(pos.x + viz.pos.x, pos.y + viz.pos.y);
}

static ImVec2 worldToVizSpace(ImVec2 pos)
{
    return ImVec2(pos.x - viz.pos.x, pos.y - viz.pos.y);
}

static void showHelp()
{
    ImGui::Begin("Help");

    ImGui::Text("LEVEL VISUALIZATION:");
    ImGui::BulletText("Click and drag a planet, food, or other object to change its position in the level");
    ImGui::BulletText("Click and drag in empty space to pan the level");
    ImGui::Separator();

    ImGui::End();
}

static void showLevelVisualization()
{
    ImGui::Begin("Level Visualization");

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 canvasStart = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 canvasEnd(canvasStart.x + canvasSize.x, canvasStart.y + canvasSize.y);

    ImGui::InvisibleButton("canvas", canvasSize);
    ImVec2 mousePosInCanvas = ImVec2(ImGui::GetIO().MousePos.x - canvasStart.x, ImGui::GetIO().MousePos.y - canvasEnd.y);

    static bool isDraggingSpace = false;
    static ImVec2 mouseDownVizPos;
    static ImVec2 mouseDownPos;

    if (ImGui::IsItemHovered() && !isDraggingSpace && ImGui::IsMouseClicked(0)) {
        isDraggingSpace = true;
        mouseDownVizPos = viz.pos;
        mouseDownPos = mousePosInCanvas;
    }

    if (isDraggingSpace)
    {
        viz.pos = ImVec2(mouseDownVizPos.x - (mousePosInCanvas.x - mouseDownPos.x),
                         mouseDownVizPos.y - (mousePosInCanvas.y - mouseDownPos.y));
        if (!ImGui::IsMouseDown(0)) isDraggingSpace = false;
    }

    ImVec2 worldStart = vizToWorldSpace(ImVec2());
    ImVec2 worldEnd = vizToWorldSpace(canvasSize);

    worldStart.x = static_cast<int>(worldStart.x / 100) * 100;
    worldStart.y = static_cast<int>(worldStart.y / 100) * 100;

    for (float x = worldStart.x; x < worldEnd.x; x += 100)
    {
        ImVec2 p1 = worldToVizSpace(ImVec2(x, 0));
        p1.x += canvasStart.x;
        p1.y = canvasStart.y;

        ImVec2 p2 = worldToVizSpace(ImVec2(x, 0));
        p2.x += canvasStart.x;
        p2.y = canvasEnd.y;

        ImU32 color = IM_COL32(50, 50, 50, 255);
        drawList->AddLine(p1, p2, color);
    }

    for (float y = worldStart.y; y < worldEnd.y; y += 100)
    {
        ImVec2 p1 = worldToVizSpace(ImVec2(0, y));
        p1.x = canvasStart.x;
        p1.y += canvasStart.y;

        ImVec2 p2 = worldToVizSpace(ImVec2(0, y));
        p2.x = canvasEnd.x;
        p2.y += canvasStart.y;

        ImU32 color = IM_COL32(50, 50, 50, 255);
        drawList->AddLine(p1, p2, color);
    }

//    ImVec2 uv0(0, 0);
//    ImVec2 uv1(0.5, 1);
//    drawList->AddImage(texture.id,
//            ImVec2(canvasPos.x - 200, canvasPos.y - 200),
//            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y));

    ImGui::End();
}

static void showDemoStuff()
{
    // Our state
    static bool show_demo_window = true;
    static bool show_another_window = false;

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin(
                "Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
//        ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

        if (ImGui::Button(
                "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
        ImGui::Begin("Another Window",
                     &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void initEditor()
{
    viz.pos = ImVec2();
    viz.zoom = 1;
    level = loadJsonLevel("examples/level1.json");
}

void runEditor()
{
    showDemoStuff();
    showHelp();
    showLevelVisualization();
}
