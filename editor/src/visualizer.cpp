#include "visualizer.h"
#include "levelmodel.h"

#include "imgui.h"
#include "util.h"
#include "global.h"

#include <memory>

static void showLevelObject(ImDrawList *drawList, const std::shared_ptr<ObjectModel>& object)
{
    float scaledWidth = object->frameSize().x * object->scale;
    float scaledHeight = object->frameSize().y * object->scale;

    ImVec2 worldTexStart(object->pos.x - scaledWidth / 2,
                         object->pos.y - scaledHeight / 2);
    ImVec2 worldTexEnd(object->pos.x + scaledWidth / 2,
                       object->pos.y + scaledHeight / 2);

    ImVec2 screenStart = g_viz.worldToScreenSpace(worldTexStart);
    ImVec2 screenEnd = g_viz.worldToScreenSpace(worldTexEnd);

    ImVec2 uv0(0, 0);
    ImVec2 uv1(object->uvEnd());

    drawList->AddImage(object->tex->id, screenStart, screenEnd, uv0, uv1);

    // Show gravity range if it's a planet
    auto planet = std::dynamic_pointer_cast<PlanetModel>(object);
    if (planet && g_showGravRanges)
    {
        // All gravity ranges seem to have this hard-coded scale at the moment...
        constexpr float GRAV_RANGE_SCALE = 3.f;

        float scaledGravWidth = g_gravRangeTex->width * object->scale / 5 * GRAV_RANGE_SCALE;
        float scaledGravHeight = g_gravRangeTex->height * object->scale * GRAV_RANGE_SCALE;

        ImVec2 worldRangeStart(object->pos.x - scaledGravWidth / 2, object->pos.y - scaledGravHeight / 2);
        ImVec2 worldRangeEnd(object->pos.x + scaledGravWidth / 2, object->pos.y + scaledGravHeight / 2);

        ImVec2 screenRangeStart = g_viz.worldToScreenSpace(worldRangeStart);
        ImVec2 screenRangeEnd = g_viz.worldToScreenSpace(worldRangeEnd);

        ImVec2 uv0(0.2, 0);
        ImVec2 uv1(0.4, 1);

        drawList->AddImage(g_gravRangeTex->id, screenRangeStart, screenRangeEnd, uv0, uv1);
    }
}

static void showLevelGrid(ImDrawList *drawList) {
    ImVec2 worldStart = g_viz.screenToWorldSpace(g_viz.getCanvas().start);
    ImVec2 worldEnd = g_viz.screenToWorldSpace(g_viz.getCanvas().end);

    constexpr float gridSpacing = 100.f;

    worldStart.x = static_cast<int>(worldStart.x / gridSpacing) * gridSpacing;
    worldStart.y = static_cast<int>(worldStart.y / gridSpacing) * gridSpacing;

    ImU32 color = IM_COL32(50, 50, 50, 255);

    for (float x = worldStart.x; x < worldEnd.x; x += gridSpacing)
    {
        ImVec2 p1 = g_viz.worldToScreenSpace(ImVec2(x, 0));
        p1.y = g_viz.getCanvas().start.y;

        ImVec2 p2 = g_viz.worldToScreenSpace(ImVec2(x, 0));
        p2.y = g_viz.getCanvas().end.y;

        drawList->AddLine(p1, p2, color);
    }

    for (float y = worldStart.y; y < worldEnd.y; y += gridSpacing)
    {
        ImVec2 p1 = g_viz.worldToScreenSpace(ImVec2(0, y));
        p1.x = g_viz.getCanvas().start.x;

        ImVec2 p2 = g_viz.worldToScreenSpace(ImVec2(0, y));
        p2.x = g_viz.getCanvas().end.x;

        drawList->AddLine(p1, p2, color);
    }
}

static std::shared_ptr<ObjectModel> findObjectAtScreenPos(ImVec2 pos)
{
    ImVec2 worldPos = g_viz.screenToWorldSpace(pos);

    auto allObjects = getAllLevelObjects(g_level);

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
            g_selectedObj = hoverObj;
            mouseDownPos = currMousePos;
            oldWorldPos = hoverObj->pos;
        }
    }
    if (isDraggingObj)
    {
        g_selectedObj->pos = ImVec2(oldWorldPos.x + currMousePos.x - mouseDownPos.x,
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
        oldVizWorldPos = g_viz.getWorldPos();
    }
    if (isDraggingSpace)
    {
        g_viz.setWorldPos(ImVec2(oldVizWorldPos.x + mouseDownScreenPos.x - currMouseScreenPos.x,
                               oldVizWorldPos.y + mouseDownScreenPos.y - currMouseScreenPos.y));
        if (!ImGui::IsMouseDown(0)) isDraggingSpace = false;
    }
}

void showLevelObjectSelection(ImDrawList *drawList)
{
    // Draw rect around selected object
    if (g_selectedObj)
    {
        auto rectColor = IM_COL32(0, 50, 180, 255);

        // TODO deduplicate this with showLevelObject()?
        float scaledWidth = g_selectedObj->frameSize().x * g_selectedObj->scale;
        float scaledHeight = g_selectedObj->frameSize().y * g_selectedObj->scale;

        ImVec2 worldTexStart(g_selectedObj->pos.x - scaledWidth / 2,
                             g_selectedObj->pos.y - scaledHeight / 2);
        ImVec2 worldTexEnd(g_selectedObj->pos.x + scaledWidth / 2,
                           g_selectedObj->pos.y + scaledHeight / 2);

        ImVec2 screenStart = g_viz.worldToScreenSpace(worldTexStart);
        ImVec2 screenEnd = g_viz.worldToScreenSpace(worldTexEnd);

        ImVec2 uv0(0, 0);
        ImVec2 uv1(g_selectedObj->uvEnd());

        drawList->AddRect(screenStart, screenEnd, rectColor, 2, ~0, 6);
    }
}

void showLevelVisualization()
{
    // Set a default size for this window for first run, in case we have no .ini
    // Necessary in this case I believe because we don't add any actual content to the window besides the
    // implicitly-sized drawing list, so it's otherwise empty
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    ImGui::Begin("Level Visualization");

    if (!g_level)
    {
        ImGui::End();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    g_viz.generateWindowCanvas();
    ImGui::InvisibleButton("canvas", g_viz.getCanvas().size); // Used to detect clicks inside window

    handleDraggingObject();
    handleDraggingSpace();
    showLevelGrid(drawList);

    for (auto& planet : g_level->planets)
    {
        showLevelObject(drawList, planet);
    }
    for (auto& food : g_level->foods)
    {
        showLevelObject(drawList, food);
    }
    showLevelObject(drawList, g_level->player);
    showLevelObject(drawList, g_level->customer);

    showLevelObjectSelection(drawList);

    ImGui::End();
}
