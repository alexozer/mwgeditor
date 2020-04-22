#pragma once

#include <imgui.h>

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

    const Canvas& getCanvas() const { return m_canvas; }

private:
    float m_zoom;
    ImVec2 m_worldPos; // Position of "camera" in world space
    Canvas m_canvas;

};

