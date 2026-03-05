#include <psp2/kernel/processmgr.h>
#include "global.h"
#include "gamepad.h"
#include "misc.h"
#include "icons.h"
#include "log.h"

using namespace Emu4VitaPlus;

#define BUTTON_INTERVAL 2
#define LEFT_POS (VITA_WIDTH / 2 - BUTTON_WIDTH * 4)
#define RIGHT_POS (VITA_WIDTH / 2 + BUTTON_WIDTH * 3)
#define CLOSE_TIME 2000000;
#define LEFT_ANGLOG_X (VITA_WIDTH / 2 - BUTTON_WIDTH * 5)
#define RIGHT_ANGLOG_X (VITA_WIDTH / 2 + BUTTON_WIDTH * 5)
#define ANGLOG_Y (BUTTON_HEIGHT * 8)
#define ANGLOG_RADIUS (BUTTON_WIDTH * 2)

Button Gamepad::_buttons[14] = {
    {BUTTON_B, {RIGHT_POS, BUTTON_HEIGHT * 3}},
    {BUTTON_Y, {RIGHT_POS - BUTTON_WIDTH, BUTTON_HEIGHT * 4}},
    {BUTTON_SELECT, {VITA_WIDTH / 2 - BUTTON_WIDTH - BUTTON_INTERVAL, BUTTON_HEIGHT * 6}},
    {BUTTON_START, {VITA_WIDTH / 2 + BUTTON_INTERVAL, BUTTON_HEIGHT * 6}},
    {KEY_UP, {LEFT_POS, BUTTON_HEIGHT * 3}},
    {KEY_DOWN, {LEFT_POS, BUTTON_HEIGHT * 5}},
    {KEY_LEFT, {LEFT_POS - BUTTON_WIDTH, BUTTON_HEIGHT * 4}},
    {KEY_RIGHT, {LEFT_POS + BUTTON_WIDTH, BUTTON_HEIGHT * 4}},
    {BUTTON_A, {RIGHT_POS + BUTTON_WIDTH, BUTTON_HEIGHT * 4}},
    {BUTTON_X, {RIGHT_POS, BUTTON_HEIGHT * 5}},
    {BUTTON_L, {LEFT_POS, BUTTON_HEIGHT * 1.6 + BUTTON_INTERVAL}, {BUTTON_WIDTH, BUTTON_HEIGHT * 0.6}},
    {BUTTON_R, {RIGHT_POS, BUTTON_HEIGHT * 1.6 + BUTTON_INTERVAL}, {BUTTON_WIDTH, BUTTON_HEIGHT * 0.6}},
    {BUTTON_L2, {LEFT_POS, BUTTON_HEIGHT}, {BUTTON_WIDTH, BUTTON_HEIGHT * 0.6}},
    {BUTTON_R2, {RIGHT_POS, BUTTON_HEIGHT}, {BUTTON_WIDTH, BUTTON_HEIGHT * 0.6}}};

Gamepad::Gamepad() : _actived(false), _start_pressed(false)
{
}

Gamepad::~Gamepad()
{
}

void Gamepad::Show()
{
    bool is_popup = ImGui::IsPopupOpen("Gamepad");
    if (_actived && !is_popup)
    {
        ImGui::OpenPopup("Gamepad");
    }

    AnalogAxis *left;
    AnalogAxis *right;
    const int16_t keys = gEmulator->GetInputInfo(left, right);
    if (keys & (1 << RETRO_DEVICE_ID_JOYPAD_START))
    {
        if (_start_pressed)
        {
            if (sceKernelGetProcessTimeWide() >= _close_time)
            {
                _actived = false;
            }
        }
        else
        {
            _start_pressed = true;
            _close_time = sceKernelGetProcessTimeWide() + CLOSE_TIME;
        }
    }
    else
    {
        _start_pressed = false;
    }

    ImGui ::SetNextWindowPos({MAIN_WINDOW_PADDING, MAIN_WINDOW_PADDING});
    ImGui::SetNextWindowSize({VITA_WIDTH, VITA_HEIGHT});
    ImGui::SetNextWindowBgAlpha(0.6);
    if (ImGui::BeginPopupModal("Gamepad", NULL, ImGuiWindowFlags_NoTitleBar))
    {
        if (!_actived && is_popup)
        {
            gEmulator->StopInput();
            ImGui::CloseCurrentPopup();
            _input->PopCallbacks();
        }

        for (size_t i = 0; i < sizeof(_buttons) / sizeof(Button); i++)
        {
            bool pressed = keys & (1 << i);
            if (pressed)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            }
            ImGui::SetCursorPos(_buttons[i].pos);
            ImGui::Button(_buttons[i].text, _buttons[i].size);
            if (pressed)
            {
                ImGui::PopStyleColor();
            }
        }

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddCircle({LEFT_ANGLOG_X, ANGLOG_Y}, ANGLOG_RADIUS, ImGui::GetColorU32(ImGuiCol_Button), 0, 5.f);
        draw_list->AddCircle({RIGHT_ANGLOG_X, ANGLOG_Y}, ANGLOG_RADIUS, ImGui::GetColorU32(ImGuiCol_Button), 0, 5.f);

        char s[255];
        snprintf(s, 255, TEXT(LANG_HOLD_EXIT), BUTTON_START);
        ImVec2 text_size = ImGui::CalcTextSize(s);
        ImGui::SetCursorScreenPos({(VITA_WIDTH - text_size.x) / 2, VITA_HEIGHT - text_size.y * 2});
        ImGui::TextUnformatted(s);
        ImGui::EndPopup();
    }
}