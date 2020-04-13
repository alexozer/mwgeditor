#include "editor.h"
#include "textures.h"
#include "loadjson.h"

#include "imgui.h"

struct Canvas
{
    ImVec2 start;
    ImVec2 end;
    ImVec2 size;
};

class VisualizationModel
{
public:
    VisualizationModel(): m_zoom{1} {}

    void generateWindowCanvas()
    {
        ImVec2 canvasStart = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasEnd(canvasStart.x + canvasSize.x, canvasStart.y + canvasSize.y);

        m_canvas = Canvas{canvasStart, canvasEnd, canvasSize};
    }

    ImVec2 vizToWorldSpace(ImVec2 vizPos) const
    {
        return ImVec2(vizPos.x + m_worldPos.x, vizPos.y + m_worldPos.y);
    }

    ImVec2 worldToVizSpace(ImVec2 worldPos) const
    {
        return ImVec2(worldPos.x - m_worldPos.x, worldPos.y - m_worldPos.y);
    }

    ImVec2 vizToScreenSpace(ImVec2 vizPos) const
    {
        ImVec2 center(m_canvas.start.x + m_canvas.size.x / 2,
                m_canvas.start.y + m_canvas.size.y / 2);
        ImVec2 screenPos(center.x + vizPos.x, center.y + vizPos.y);
        return screenPos;
    }

    ImVec2 screenToVizSpace(ImVec2 screenPos) const
    {
        ImVec2 center(m_canvas.start.x + m_canvas.size.x / 2,
                      m_canvas.start.y + m_canvas.size.y / 2);
        ImVec2 vizPos(screenPos.x - center.x, screenPos.y - center.y);
        return vizPos;
    }

    ImVec2 worldToScreenSpace(ImVec2 worldPos) const
    {
        return vizToScreenSpace(worldToVizSpace(worldPos));
    }

    ImVec2 screenToWorldSpace(ImVec2 screenPos) const
    {
        return vizToWorldSpace(screenToVizSpace(screenPos));
    }

    ImVec2 getWorldPos() const { return m_worldPos; }

    void setWorldPos(ImVec2 worldPos) { m_worldPos = worldPos; }

    const Canvas &getCanvas() const { return m_canvas; }

private:
    float m_zoom;
    ImVec2 m_worldPos; // Position of "camera" in world space
    Canvas m_canvas;

};

static std::shared_ptr<LevelModel> level = {};
static VisualizationModel viz = {};
static std::shared_ptr<ObjectModel> selectedObj = nullptr;

static void showHelp()
{
    ImGui::Begin("Help");

    ImGui::Text("LEVEL VISUALIZATION:");
    ImGui::BulletText("Click and drag a planet, food, or other object to change its position in the level");
    ImGui::BulletText("Click and drag in empty space to pan the level");
    ImGui::Separator();

    ImGui::End();
}

static void showLevelObject(ImDrawList *drawList, const std::shared_ptr<ObjectModel>& object)
{
    float scaledWidth = object->frameSize().x * object->scale;
    float scaledHeight = object->frameSize().y * object->scale;

    ImVec2 worldTexStart(object->pos.x - scaledWidth / 2,
                         object->pos.y - scaledHeight / 2);
    ImVec2 worldTexEnd(object->pos.x + scaledWidth / 2,
                       object->pos.y + scaledHeight / 2);

    ImVec2 screenStart = viz.worldToScreenSpace(worldTexStart);
    ImVec2 screenEnd = viz.worldToScreenSpace(worldTexEnd);

    ImVec2 uv0(0, 0);
    ImVec2 uv1(object->uvEnd());

    drawList->AddImage(object->tex.id, screenStart, screenEnd, uv0, uv1);
}

static void showLevelGrid(ImDrawList *drawList) {
    ImVec2 worldStart = viz.screenToWorldSpace(viz.getCanvas().start);
    ImVec2 worldEnd = viz.screenToWorldSpace(viz.getCanvas().end);

    constexpr float gridSpacing = 100.f;

    worldStart.x = static_cast<int>(worldStart.x / gridSpacing) * gridSpacing;
    worldStart.y = static_cast<int>(worldStart.y / gridSpacing) * gridSpacing;

    ImU32 color = IM_COL32(50, 50, 50, 255);

    for (float x = worldStart.x; x < worldEnd.x; x += gridSpacing)
    {
        ImVec2 p1 = viz.worldToScreenSpace(ImVec2(x, 0));
        p1.y = viz.getCanvas().start.y;

        ImVec2 p2 = viz.worldToScreenSpace(ImVec2(x, 0));
        p2.y = viz.getCanvas().end.y;

        drawList->AddLine(p1, p2, color);
    }

    for (float y = worldStart.y; y < worldEnd.y; y += gridSpacing)
    {
        ImVec2 p1 = viz.worldToScreenSpace(ImVec2(0, y));
        p1.x = viz.getCanvas().start.x;

        ImVec2 p2 = viz.worldToScreenSpace(ImVec2(0, y));
        p2.x = viz.getCanvas().end.x;

        drawList->AddLine(p1, p2, color);
    }
}

static std::shared_ptr<ObjectModel> findObjectAtScreenPos(ImVec2 pos)
{
    ImVec2 worldPos = viz.screenToWorldSpace(pos);

    std::vector<std::shared_ptr<ObjectModel>> allObjects;
    allObjects.insert(allObjects.end(), level->planets.begin(), level->planets.end());
    allObjects.insert(allObjects.end(), level->foods.begin(), level->foods.end());
    allObjects.emplace_back(level->customer);
    allObjects.emplace_back(level->player);

    for (auto obj : allObjects) {
        float scaledWidth = obj->frameSize().x * obj->scale;
        float scaledHeight = obj->frameSize().y * obj->scale;

        if (worldPos.x >= (obj->pos.x - scaledWidth / 2) &&
            worldPos.y >= (obj->pos.y - scaledHeight / 2) &&
            worldPos.x <= (obj->pos.x + scaledWidth / 2) &&
            worldPos.y <= (obj->pos.y + scaledHeight / 2))
        {
            return obj;
        }
    }

    return nullptr;
}

static void handleDraggingObject()
{
    // TODO dedup with handleDraggingSpace()?

    static bool isDraggingObj = false;
    static ImVec2 mouseDownPos;
    static ImVec2 oldWorldPos;

    ImVec2 currMousePos = ImGui::GetIO().MousePos;

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !isDraggingObj)
    {
        auto hoverObj = findObjectAtScreenPos(currMousePos);
        if (hoverObj)
        {
            isDraggingObj = true;
            selectedObj = hoverObj;
            mouseDownPos = currMousePos;
            oldWorldPos = hoverObj->pos;
        }
    }
    if (isDraggingObj)
    {
        selectedObj->pos = ImVec2(oldWorldPos.x + currMousePos.x - mouseDownPos.x,
                                  oldWorldPos.y + currMousePos.y - mouseDownPos.y);
        if (!ImGui::IsMouseDown(0)) isDraggingObj = false;
    }
}

static void handleDraggingSpace()
{
    static bool isDraggingSpace = false;
    static ImVec2 mouseDownScreenPos;
    static ImVec2 oldVizWorldPos;

    ImVec2 currMouseScreenPos = ImGui::GetIO().MousePos;

    if (ImGui::IsItemHovered() && !isDraggingSpace && ImGui::IsMouseClicked(0) && !findObjectAtScreenPos(currMouseScreenPos))
    {
        isDraggingSpace = true;
        mouseDownScreenPos = currMouseScreenPos;
        oldVizWorldPos = viz.getWorldPos();
    }
    if (isDraggingSpace)
    {
        viz.setWorldPos(ImVec2(oldVizWorldPos.x + mouseDownScreenPos.x - currMouseScreenPos.x,
                               oldVizWorldPos.y + mouseDownScreenPos.y - currMouseScreenPos.y));
        if (!ImGui::IsMouseDown(0)) isDraggingSpace = false;
    }
}

void showLevelObjectSelection(ImDrawList *drawList)
{
    // Draw rect around selected object
    if (selectedObj)
    {
        auto rectColor = IM_COL32(0, 50, 180, 255);

        // TODO deduplicate this with showLevelObject()?
        float scaledWidth = selectedObj->frameSize().x * selectedObj->scale;
        float scaledHeight = selectedObj->frameSize().y * selectedObj->scale;

        ImVec2 worldTexStart(selectedObj->pos.x - scaledWidth / 2,
                             selectedObj->pos.y - scaledHeight / 2);
        ImVec2 worldTexEnd(selectedObj->pos.x + scaledWidth / 2,
                           selectedObj->pos.y + scaledHeight / 2);

        ImVec2 screenStart = viz.worldToScreenSpace(worldTexStart);
        ImVec2 screenEnd = viz.worldToScreenSpace(worldTexEnd);

        ImVec2 uv0(0, 0);
        ImVec2 uv1(selectedObj->uvEnd());

        drawList->AddRect(screenStart, screenEnd, rectColor, 2, ~0, 6);
    }

}

static void showLevelVisualization()
{
    ImGui::Begin("Level Visualization");

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    viz.generateWindowCanvas();
    ImGui::InvisibleButton("canvas", viz.getCanvas().size); // Used to detect clicks inside window

    handleDraggingObject();
    handleDraggingSpace();
    showLevelGrid(drawList);

    for (auto& planet : level->planets)
    {
        showLevelObject(drawList, planet);
    }
    for (auto& food : level->foods)
    {
        showLevelObject(drawList, food);
    }
    showLevelObject(drawList, level->player);
    showLevelObject(drawList, level->customer);

    showLevelObjectSelection(drawList);

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
    level = loadJsonLevel("examples/level1.json");

    // Initially center on the start planet
    // Or else the initial position is (0, 0) I guess
    for (auto& planet : level->planets)
    {
        if (planet->order == PlanetOrder::START)
        {
            viz.setWorldPos(planet->pos);
            selectedObj = planet;
        }
    }
}

void runEditor()
{
    showDemoStuff();
    showHelp();
    showLevelVisualization();
}
