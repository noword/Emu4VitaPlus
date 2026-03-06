#include <psp2/kernel/processmgr.h>
#include "global.h"
#include "gamepad.h"
#include "misc.h"
#include "icons.h"
#include "log.h"

using namespace Emu4VitaPlus;

#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 50
#define BUTTON_INTERVAL 2
#define LEFT_POS (VITA_WIDTH / 2 - BUTTON_WIDTH * 4)
#define RIGHT_POS (VITA_WIDTH / 2 + BUTTON_WIDTH * 3)
#define CLOSE_TIME 2000000;
#define LEFT_ANALOG_X (VITA_WIDTH / 2 - BUTTON_WIDTH * 5)
#define RIGHT_ANALOG_X (VITA_WIDTH / 2 + BUTTON_WIDTH * 5)
#define ANALOG_Y (BUTTON_HEIGHT * 8)
#define ANALOG_RADIUS (BUTTON_WIDTH * 2)

Button Gamepad::_buttons[14] = {
    {BUTTON_B, {RIGHT_POS, BUTTON_HEIGHT * 3.5}},
    {BUTTON_Y, {RIGHT_POS - BUTTON_WIDTH / 2, BUTTON_HEIGHT * 4}},
    {BUTTON_SELECT, {VITA_WIDTH / 2 - BUTTON_WIDTH - BUTTON_INTERVAL, BUTTON_HEIGHT * 6}},
    {BUTTON_START, {VITA_WIDTH / 2 + BUTTON_INTERVAL, BUTTON_HEIGHT * 6}},
    {KEY_UP, {0, 0}},    // not used
    {KEY_DOWN, {0, 0}},  // not used
    {KEY_LEFT, {0, 0}},  // not used
    {KEY_RIGHT, {0, 0}}, // not used
    {BUTTON_A, {RIGHT_POS + BUTTON_WIDTH / 2, BUTTON_HEIGHT * 4}},
    {BUTTON_X, {RIGHT_POS, BUTTON_HEIGHT * 4.5}},
    {BUTTON_L, {LEFT_POS, BUTTON_HEIGHT * 1.6 + BUTTON_INTERVAL}},
    {BUTTON_R, {RIGHT_POS, BUTTON_HEIGHT * 1.6 + BUTTON_INTERVAL}},
    {BUTTON_L2, {LEFT_POS, BUTTON_HEIGHT}},
    {BUTTON_R2, {RIGHT_POS, BUTTON_HEIGHT}}};

const char *DPADS[16] = {
    BUTTON_EMPTY_DPAD, // 0
    BUTTON_UP,         // 1
    BUTTON_DOWN,       // 2
    BUTTON_EMPTY_DPAD, // 3
    BUTTON_LEFT,       // 4
    BUTTON_LEFT_UP,    // 5
    BUTTON_LEFT_DOWN,  // 6
    BUTTON_EMPTY_DPAD, // 7
    BUTTON_RIGHT,      // 8
    BUTTON_RIGHT_UP,   // 9
    BUTTON_RIGHT_DOWN, // A
    BUTTON_EMPTY_DPAD, // B
    BUTTON_EMPTY_DPAD, // C
    BUTTON_EMPTY_DPAD, // D
    BUTTON_EMPTY_DPAD  // E
};

// square to circle
static void CoorMap(AnalogAxis axis, float *x, float *y)
{
    float u = (axis.x - 127.5f) / 127.5f;
    float v = (axis.y - 127.5f) / 127.5f;

    *x = u * sqrtf(1.f - v * v * 0.5) * ANALOG_RADIUS;
    *y = v * sqrtf(1.f - u * u * 0.5) * ANALOG_RADIUS;
}

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

    AnalogAxis left;
    AnalogAxis right;
    const int16_t keys = gEmulator->GetInputInfo(left, right);
    if (keys & (1 << RETRO_DEVICE_ID_JOYPAD_START))
    {
        if (_start_pressed)
        {
            if (sceKernelGetProcessTimeWide() >= _close_time)
            {
                _start_pressed = false;
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

    ImGui ::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({VITA_WIDTH, VITA_HEIGHT});
    ImGui::SetNextWindowBgAlpha(0.6);
    if (ImGui::BeginPopupModal("Gamepad", NULL, ImGuiWindowFlags_NoTitleBar))
    {
        if (!_actived && is_popup)
        {
            if (gStatus.Get() != APP_STATUS_SHOW_UI_IN_GAME)
                gEmulator->StopInput();
            ImGui::CloseCurrentPopup();
            _input->PopCallbacks();
        }

        ImGui::PushFont(gLargeFont);

        for (size_t i = 0; i < sizeof(_buttons) / sizeof(Button); i++)
        {
            if (i >= RETRO_DEVICE_ID_JOYPAD_UP && i <= RETRO_DEVICE_ID_JOYPAD_RIGHT)
                continue;

            bool pressed = keys & (1 << i);
            if (pressed)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            }
            ImGui::SetCursorPos(_buttons[i].pos);
            ImGui::TextUnformatted(_buttons[i].text);
            // ImGui::Button(_buttons[i].text, _buttons[i].size);
            if (pressed)
            {
                ImGui::PopStyleColor();
            }
        }

        const char *dpad = DPADS[(keys >> 4) & 0xf];
        ImGui::SetCursorPos({LEFT_POS, BUTTON_HEIGHT * 4});
        if (dpad != BUTTON_EMPTY_DPAD)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        }
        ImGui::TextUnformatted(dpad);
        if (dpad != BUTTON_EMPTY_DPAD)
        {
            ImGui::PopStyleColor();
        }

        ImGui::PopFont();

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddCircle({LEFT_ANALOG_X, ANALOG_Y}, ANALOG_RADIUS, ImGui::GetColorU32(ImGuiCol_Button), 0, 5.f);
        draw_list->AddCircle({RIGHT_ANALOG_X, ANALOG_Y}, ANALOG_RADIUS, ImGui::GetColorU32(ImGuiCol_Button), 0, 5.f);

        float left_x, left_y, right_x, right_y;
        CoorMap(left, &left_x, &left_y);
        CoorMap(right, &right_x, &right_y);

        ImU32 left_color = (keys & (1 << RETRO_DEVICE_ID_JOYPAD_L3)) ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Text);
        ImU32 right_color = (keys & (1 << RETRO_DEVICE_ID_JOYPAD_R3)) ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Text);

        draw_list->AddCircleFilled({LEFT_ANALOG_X + left_x, ANALOG_Y + left_y}, BUTTON_WIDTH / 2, left_color);
        draw_list->AddCircleFilled({RIGHT_ANALOG_X + right_x, ANALOG_Y + right_y}, BUTTON_WIDTH / 2, right_color);

        char s[255];
        snprintf(s, 255, TEXT(LANG_HOLD_EXIT), BUTTON_START);
        ImVec2 text_size = ImGui::CalcTextSize(s);
        ImGui::SetCursorScreenPos({(VITA_WIDTH - text_size.x) / 2, VITA_HEIGHT - text_size.y * 2});
        ImGui::TextUnformatted(s);
        ImGui::EndPopup();
    }
}