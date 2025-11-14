#include <psp2/kernel/processmgr.h>
#include <string>
#include "fps.h"
#include "global.h"
#include "my_imgui.h"

#define FPS_SAMPLES 32
#define ONE_SECOND 1000000

#define FPS_HUD_WIDTH 50
#define FPS_HUD_HEIGHT 30

#define GREEN_THRESHOLD int(60 * 0.8)
#define YELLOW_THRESHOLD int(60 * 0.6)

Fps::Fps() : _fps(0)
{
    _last_time = sceKernelGetProcessTimeWide();
}

Fps::~Fps()
{
}

void Fps::Update()
{
    _frame_count++;
    if (_frame_count >= FPS_SAMPLES)
    {
        uint64_t now = sceKernelGetProcessTimeWide();
        uint64_t delta = now - _last_time;
        _fps = ONE_SECOND / (delta / FPS_SAMPLES);
        _last_time = now;
        _frame_count = 0;
    }
}

void Fps::Show()
{
    ImVec2 pos;
    switch (gConfig->fps)
    {
    case FPS_POSITION_TOP_LEFT:
        pos = {0.f, 0.f};
        break;

    case FPS_POSITION_TOP_RIGHT:
        pos = {VITA_WIDTH - FPS_HUD_WIDTH, 0.f};
        break;

    case FPS_POSITION_BOTTOM_LEFT:
        pos = {0.f, VITA_HEIGHT - FPS_HUD_HEIGHT};
        break;

    case FPS_POSITION_BOTTOM_RIGHT:
        break;

    default:
        return;
    }

    ImU32 color;
    if (_fps > GREEN_THRESHOLD)
        color = IM_COL32(0, 255, 0, 255);
    else if (_fps > YELLOW_THRESHOLD)
        color = IM_COL32(255, 255, 0, 255);
    else
        color = IM_COL32(255, 0, 0, 255);

#ifndef DRAW_IMGUI_TOGETHER
    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
#endif
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize({FPS_HUD_WIDTH, FPS_HUD_HEIGHT});
    ImGui::SetNextWindowBgAlpha(0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::Begin("fps", NULL,
                     ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(std::to_string(_fps).c_str());
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();

#ifndef DRAW_IMGUI_TOGETHER
    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
#endif
}