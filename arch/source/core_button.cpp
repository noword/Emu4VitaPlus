#include "defines.h"
#include "core_button.h"
#include "log.h"
#include "utils.h"
#include "file.h"
#include "app.h"
#include "config.h"
#include "language_arch.h"
#include "icons.h"

#define DISABLE_COLOR 0.5f
#define DELTA_COLOR ((1.f - DISABLE_COLOR) / GRADIENT_FRAMES)

CoreButton::CoreButton(CONSOLE console, std::vector<CoreName> cores)
    : _console(console),
      _cores(std::move(cores)),
      _actived(false),
      _index(0),
      _tint_color{DISABLE_COLOR, DISABLE_COLOR, DISABLE_COLOR, DISABLE_COLOR},
      _bg_color{0, 0, 0, 0}
{
    std::string path = std::string(CORE_DATA_DIR) + "/" + CONSOLE_NAMES[_console];
    _button_texture = vita2d_load_PNG_file((path + "/icon0.png").c_str());
    _cover_texture = vita2d_load_PNG_file((path + "/console.png").c_str());
    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
    _delta_color = {
        (color.x - _bg_color.x) / GRADIENT_FRAMES,
        (color.y - _bg_color.y) / GRADIENT_FRAMES,
        (color.z - _bg_color.z) / GRADIENT_FRAMES,
        (color.w - _bg_color.w) / GRADIENT_FRAMES,
    };
}

CoreButton::~CoreButton()
{
    vita2d_free_texture(_button_texture);
    vita2d_free_texture(_cover_texture);
}

void CoreButton::Show(bool selected, bool choice)
{
    if (selected)
    {
        if (_tint_color.x < 1.f)
        {
            _tint_color.x += DELTA_COLOR;
            _tint_color.y += DELTA_COLOR;
            _tint_color.z += DELTA_COLOR;
            _tint_color.w += DELTA_COLOR;
            _bg_color.x += _delta_color.x;
            _bg_color.y += _delta_color.y;
            _bg_color.z += _delta_color.z;
            _bg_color.w += _delta_color.w;
        }
    }
    else
    {
        _tint_color = {DISABLE_COLOR, DISABLE_COLOR, DISABLE_COLOR, DISABLE_COLOR};
        _bg_color = {0, 0, 0, 0};
    }

    ImGui::ImageButton(_button_texture,
                       {BUTTON_SIZE, BUTTON_SIZE},
                       {0.f, 0.f},
                       {1.f, 1.f},
                       0,
                       _bg_color,
                       _tint_color);

    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::CalcTextSize(CONSOLE_NAMES[_console]);

    pos.x += (BUTTON_SIZE - size.x) / 2;
    pos.y += BUTTON_SIZE - size.y - 5;
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImU32 color;
    if (choice)
    {
        color = gConfig->consoles[_console] ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
    }
    else
    {
        color = selected ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
    }

    draw_list->AddText(pos, color, CONSOLE_NAMES[_console]);

    if (selected)
    {
        _ShowPopup();
    }

    if (choice)
    {
        pos = ImGui::GetItemRectMin();
        if (gConfig->consoles[_console])
        {
            draw_list->AddText(pos, IM_COL32_WHITE, ICON_ON);
        }
        else
        {
            draw_list->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), ICON_OFF);
        }
    }
}

void CoreButton::_ShowPopup()
{
    bool is_popup = ImGui::IsPopupOpen(CONSOLE_NAMES[_console]);

    if (_actived && !is_popup)
    {
        ImGui::OpenPopup(CONSOLE_NAMES[_console]);
    }

    if (ImGui::BeginPopupModal(CONSOLE_NAMES[_console], NULL, ImGuiWindowFlags_NoTitleBar))
    {
        if (!_actived && is_popup)
        {
            ImGui::CloseCurrentPopup();
        }
        size_t count = 0;
        for (const auto &core : _cores)
        {
            ImGui::Selectable(core.name.c_str(), count == _index);
            count++;
        }
        ImGui::EndPopup();
    }
}

void CoreButton::OnActive(Input *input)
{
    _actived = true;
    input->PushCallbacks();
    SetInputHooks(input);
}

void CoreButton::_OnKeyUp(Input *input)
{
    LOOP_MINUS_ONE(_index, _cores.size());
}

void CoreButton::_OnKeyDown(Input *input)
{
    LOOP_PLUS_ONE(_index, _cores.size());
}

void CoreButton::_OnClick(Input *input)
{
    _actived = false;
    input->PopCallbacks();
    _BootCore();
}

void CoreButton::_OnCancel(Input *input)
{
    _actived = false;
    input->PopCallbacks();
}

void CoreButton::_BootCore()
{
    LogFunctionName;
    if (_cores.size() == 0)
    {
        return;
    }

    gConfig->last_core = _cores[_index].boot_name;
    gCoreName = _cores[_index].boot_name;
    gRunning = false;
}

void CoreButton::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&CoreButton::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&CoreButton::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_UP, std::bind(&CoreButton::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_DOWN, std::bind(&CoreButton::_OnKeyDown, this, input), true);
    input->SetKeyUpCallback(EnterButton, std::bind(&CoreButton::_OnClick, this, input));
    input->SetKeyUpCallback(CancelButton, std::bind(&CoreButton::_OnCancel, this, input));
}

void CoreButton::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_DOWN);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

const char *CoreButton::GetIntro() const
{
    return gArchs[gConfig->language][_console];
}