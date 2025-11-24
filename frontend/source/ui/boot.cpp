#include "global.h"
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
    if (ImGui::BeginChild("ChildAbout", {0, 0}, false))
    {
        size_t count = 0;
        for (const auto &log : _logs)
        {
            if (_show_spin && &log == &_logs.back())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_GREEN);
            }
            ImGui::TextUnformatted(log.c_str());

            if (count == _index && ImGui::GetScrollMaxY() > 0.f)
            {
                size_t total = _logs.size();
                if (_show_spin)
                {
                    total++;
                }
                ImGui::SetScrollHereY((float)_index / (float)total);
            }

            count++;
        }

        if (_show_spin && _logs.size() > 0)
        {
            ImGui::PopStyleColor();
        }

        if (_show_spin)
        {
            _spin_text.Show();
        }
    }

    ImGui::EndChild();
}

void Boot::AppendLog(const char *log)
{
    LogInfo("boot log: %s", log);

    gVideo->Lock();
    _logs.emplace_back(log);
    _index = _logs.size() - 1;
    gVideo->Unlock();

    _show_spin = true;
}

void Boot::ClearLogs()
{
    LogFunctionName;
    gVideo->Lock();
    _logs.clear();
    _index = 0;
    gVideo->Unlock();
}

void Boot::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&Boot::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&Boot::_OnKeyDown, this, input), true);
    input->SetKeyUpCallback(SCE_CTRL_TRIANGLE, std::bind(&Boot::_OnKey, this, input));
    input->SetKeyUpCallback(SCE_CTRL_SQUARE, std::bind(&Boot::_OnKey, this, input));
    input->SetKeyUpCallback(SCE_CTRL_CIRCLE, std::bind(&Boot::_OnKey, this, input));
    input->SetKeyUpCallback(SCE_CTRL_CROSS, std::bind(&Boot::_OnKey, this, input));

    _show_spin = false;
}

void Boot::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_TRIANGLE);
    input->UnsetKeyDownCallback(SCE_CTRL_SQUARE);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

void Boot::_OnKey(Input *input)
{
    LogFunctionName;
    UnsetInputHooks(input);
    ClearLogs();
    gStatus.Set(APP_STATUS_SHOW_UI);
}

void Boot::_OnKeyUp(Input *input)
{
    LogFunctionName;
    if (_index > 0)
        _index--;
}

void Boot::_OnKeyDown(Input *input)
{
    LogFunctionName;
    if (_index + 1 < _logs.size())
        _index++;
}