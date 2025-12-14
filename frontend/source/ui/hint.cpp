#include <imgui_vita2d/imgui_vita.h>
#include "my_imgui.h"
#include "hint.h"
#include "misc.h"

Hint::Hint()
{
}

Hint::~Hint()
{
}

void Hint::Show()
{
    if (_hints.empty())
    {
        return;
    }

    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    _locker.Lock();
    HintItem &hint = _hints.front();

    const char *s = hint.lang_string.Get();
    ImVec2 size = ImGui::CalcTextSize(s);
    float x = (VITA_WIDTH - size.x) / 2;
    float y = VITA_HEIGHT - size.y - MAIN_WINDOW_PADDING;

    ImGui::SetNextWindowPos({x - MAIN_WINDOW_PADDING, y - MAIN_WINDOW_PADDING});
    ImGui::SetNextWindowSize({size.x + MAIN_WINDOW_PADDING * 2, size.y + MAIN_WINDOW_PADDING * 2});
    ImGui::SetNextWindowBgAlpha(0.8);

    if (ImGui::Begin("hint", NULL,
                     ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::SetCursorPos({10, 10});
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::TextWrapped(s);
        ImGui::PopStyleColor();
    }
    ImGui::End();

    hint.frame_count--;
    if (hint.frame_count == 0)
    {
        _hints.pop();
    }
    _locker.Unlock();

    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
}

void Hint::SetHint(LanguageString s, int frame_count, bool clear_exists)
{
    _locker.Lock();
    if (clear_exists)
    {
        while (!_hints.empty())
            _hints.pop();
    }
    _hints.push({s, frame_count});
    _locker.Unlock();
}
