#include "video.h"
#include "app.h"
#include "boot.h"
#include "utils.h"

Boot::Boot() : _index(0)
{
}

Boot::~Boot()
{
}

void Boot::Show()
{
    for (const auto &log : _logs)
    {
        if (&log == &_logs.back())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_GREEN);
        }
        ImGui::Text(log.c_str());
    }

    if (_logs.size() > 0)
    {
        ImGui::PopStyleColor();
    }

    _spin_text.Show();

    if (ImGui::GetScrollMaxY() > 0.f)
    {
        ImGui::SetScrollHereY((float)_index / (float)_logs.size());
    }
}

void Boot::Run()
{
    _input.Poll(true);
}

void Boot::AppendLog(const char *log)
{
    LogInfo("boot log: %s", log);

    gVideo->Lock();
    _logs.emplace_back(log);
    _index = _logs.size() - 1;
    gVideo->Unlock();
}

void Boot::ClearLogs()
{
    LogFunctionName;
    gVideo->Lock();
    _logs.clear();
    _index = 0;
    gVideo->Unlock();
}

void Boot::SetInputHooks()
{
    _input.SetKeyDownCallback(SCE_CTRL_UP, std::bind(&Boot::_OnKeyUp, this, &_input), true);
    _input.SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&Boot::_OnKeyDown, this, &_input), true);
    _input.SetKeyUpCallback(SCE_CTRL_TRIANGLE, std::bind(&Boot::_OnKey, this, &_input));
    _input.SetKeyUpCallback(SCE_CTRL_SQUARE, std::bind(&Boot::_OnKey, this, &_input));
    _input.SetKeyUpCallback(SCE_CTRL_CIRCLE, std::bind(&Boot::_OnKey, this, &_input));
    _input.SetKeyUpCallback(SCE_CTRL_CROSS, std::bind(&Boot::_OnKey, this, &_input));
}

void Boot::UnsetInputHooks()
{
    _input.UnsetKeyDownCallback(SCE_CTRL_UP);
    _input.UnsetKeyDownCallback(SCE_CTRL_DOWN);
    _input.UnsetKeyDownCallback(SCE_CTRL_TRIANGLE);
    _input.UnsetKeyDownCallback(SCE_CTRL_SQUARE);
    _input.UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    _input.UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

void Boot::_OnKey(Input *input)
{
    gStatus.Set(APP_STATUS_SHOW_UI);
}

void Boot::_OnKeyUp(Input *input)
{
    LOOP_MINUS_ONE(_index, _logs.size());
}

void Boot::_OnKeyDown(Input *input)
{
    LOOP_PLUS_ONE(_index, _logs.size());
}