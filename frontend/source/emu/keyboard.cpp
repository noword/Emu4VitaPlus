#include "my_imgui.h"
#include "keyboard.h"
#include "icons.h"
#include "log.h"

using namespace Emu4VitaPlus;

#define KEY_BACKSPACE_WIDTH (KEY_BUTTON_WIDTH * 2 + KEY_BUTTON_INTERVAL)
#define KEY_TAB_WIDTH (KEY_BUTTON_WIDTH * 1.5)
#define KEY_CAPSLOCK_WIDTH (KEY_BUTTON_WIDTH * 1.8)
#define KEY_RETURN_WIDTH (KEY_BUTTON_WIDTH * 2.2 + KEY_BUTTON_INTERVAL * 2)
#define KEY_LSHIFT_WIDTH (KEY_BUTTON_WIDTH * 2.4)
#define KEY_RSHIFT_WIDTH (KEY_BUTTON_WIDTH * 2.6 + KEY_BUTTON_INTERVAL * 3)
#define KEY_BOTTOM_WIDTH (KEY_BUTTON_WIDTH * 1.2)
#define KEY_SPACE_WIDTH (KEY_BUTTON_WIDTH * 8 + KEY_BUTTON_INTERVAL * 3)

#define X_START 0

#define X_ESC X_START
#define X_F1 (X_ESC + KEY_BUTTON_NEXT * 2)
#define X_F2 (X_F1 + KEY_BUTTON_NEXT)
#define X_F3 (X_F2 + KEY_BUTTON_NEXT)
#define X_F4 (X_F3 + KEY_BUTTON_NEXT)
#define X_F5 (X_F4 + KEY_BUTTON_NEXT * 1.5)
#define X_F6 (X_F5 + KEY_BUTTON_NEXT)
#define X_F7 (X_F6 + KEY_BUTTON_NEXT)
#define X_F8 (X_F7 + KEY_BUTTON_NEXT)
#define X_F9 (X_F8 + KEY_BUTTON_NEXT * 1.5)
#define X_F10 (X_F9 + KEY_BUTTON_NEXT)
#define X_F11 (X_F10 + KEY_BUTTON_NEXT)
#define X_F12 (X_F11 + KEY_BUTTON_NEXT)
#define X_EXT0 (X_F12 + KEY_BUTTON_WIDTH * 1.5)
#define X_EXT1 (X_EXT0 + KEY_BUTTON_NEXT)
#define X_EXT2 (X_EXT1 + KEY_BUTTON_NEXT)

#define X_BACKQUOTE X_START
#define X_1 (X_BACKQUOTE + KEY_BUTTON_NEXT)
#define X_2 (X_1 + KEY_BUTTON_NEXT)
#define X_3 (X_2 + KEY_BUTTON_NEXT)
#define X_4 (X_3 + KEY_BUTTON_NEXT)
#define X_5 (X_4 + KEY_BUTTON_NEXT)
#define X_6 (X_5 + KEY_BUTTON_NEXT)
#define X_7 (X_6 + KEY_BUTTON_NEXT)
#define X_8 (X_7 + KEY_BUTTON_NEXT)
#define X_9 (X_8 + KEY_BUTTON_NEXT)
#define X_0 (X_9 + KEY_BUTTON_NEXT)
#define X_MINUS (X_0 + KEY_BUTTON_NEXT)
#define X_EQUALS (X_MINUS + KEY_BUTTON_NEXT)
#define X_BACKSPACE (X_EQUALS + KEY_BUTTON_NEXT)

#define X_TAB X_START
#define X_Q (X_TAB + KEY_TAB_WIDTH + KEY_BUTTON_INTERVAL)
#define X_W (X_Q + KEY_BUTTON_NEXT)
#define X_E (X_W + KEY_BUTTON_NEXT)
#define X_R (X_E + KEY_BUTTON_NEXT)
#define X_T (X_R + KEY_BUTTON_NEXT)
#define X_Y (X_T + KEY_BUTTON_NEXT)
#define X_U (X_Y + KEY_BUTTON_NEXT)
#define X_I (X_U + KEY_BUTTON_NEXT)
#define X_O (X_I + KEY_BUTTON_NEXT)
#define X_P (X_O + KEY_BUTTON_NEXT)
#define X_LEFTBRACKET (X_P + KEY_BUTTON_NEXT)
#define X_RIGHTBRACKET (X_LEFTBRACKET + KEY_BUTTON_NEXT)
#define X_BACKSLASH (X_RIGHTBRACKET + KEY_BUTTON_NEXT)

#define X_CAPSLOCK X_START
#define X_A (X_CAPSLOCK + KEY_CAPSLOCK_WIDTH + KEY_BUTTON_INTERVAL)
#define X_S (X_A + KEY_BUTTON_NEXT)
#define X_D (X_S + KEY_BUTTON_NEXT)
#define X_F (X_D + KEY_BUTTON_NEXT)
#define X_G (X_F + KEY_BUTTON_NEXT)
#define X_H (X_G + KEY_BUTTON_NEXT)
#define X_J (X_H + KEY_BUTTON_NEXT)
#define X_K (X_J + KEY_BUTTON_NEXT)
#define X_L (X_K + KEY_BUTTON_NEXT)
#define X_SEMICOLON (X_L + KEY_BUTTON_NEXT)
#define X_QUOTE (X_SEMICOLON + KEY_BUTTON_NEXT)
#define X_RETURN (X_QUOTE + KEY_BUTTON_NEXT)

#define X_LSHIFT X_START
#define X_Z (X_LSHIFT + KEY_LSHIFT_WIDTH + KEY_BUTTON_INTERVAL)
#define X_X (X_Z + KEY_BUTTON_NEXT)
#define X_C (X_X + KEY_BUTTON_NEXT)
#define X_V (X_C + KEY_BUTTON_NEXT)
#define X_B (X_V + KEY_BUTTON_NEXT)
#define X_N (X_B + KEY_BUTTON_NEXT)
#define X_M (X_N + KEY_BUTTON_NEXT)
#define X_COMMA (X_M + KEY_BUTTON_NEXT)
#define X_PERIOD (X_COMMA + KEY_BUTTON_NEXT)
#define X_SLASH (X_PERIOD + KEY_BUTTON_NEXT)
#define X_RSHIFT (X_SLASH + KEY_BUTTON_NEXT)

#define X_LCTRL X_START
#define X_LMETA (X_LCTRL + KEY_BOTTOM_WIDTH + KEY_BUTTON_INTERVAL)
#define X_LALT (X_LMETA + KEY_BOTTOM_WIDTH + KEY_BUTTON_INTERVAL)
#define X_SPACE (X_LALT + KEY_BOTTOM_WIDTH + KEY_BUTTON_INTERVAL)
#define X_RALT (X_SPACE + KEY_SPACE_WIDTH + KEY_BUTTON_INTERVAL)
#define X_RMETA (X_RALT + KEY_BOTTOM_WIDTH + KEY_BUTTON_INTERVAL)
#define X_RCTRL (X_RMETA + KEY_BOTTOM_WIDTH + KEY_BUTTON_INTERVAL)
#define X_LEFT (X_RCTRL + KEY_BUTTON_WIDTH * 1.5)

#define Y_0 0
#define Y_1 (Y_0 + KEY_BUTTON_HEIGHT + KEY_BUTTON_INTERVAL)
#define Y_2 (Y_1 + KEY_BUTTON_HEIGHT + KEY_BUTTON_INTERVAL)
#define Y_3 (Y_2 + KEY_BUTTON_HEIGHT + KEY_BUTTON_INTERVAL)
#define Y_4 (Y_3 + KEY_BUTTON_HEIGHT + KEY_BUTTON_INTERVAL)
#define Y_5 (Y_4 + KEY_BUTTON_HEIGHT + KEY_BUTTON_INTERVAL)

const KeyButton Keyboard::_buttons[] = {
    KeyButton{{RETROK_ESCAPE, "Esc"}, {X_ESC, Y_0}},
    KeyButton{{RETROK_F1, "F1"}, {X_F1, Y_0}},
    KeyButton{{RETROK_F2, "F2"}, {X_F2, Y_0}},
    KeyButton{{RETROK_F3, "F3"}, {X_F3, Y_0}},
    KeyButton{{RETROK_F4, "F4"}, {X_F4, Y_0}},
    KeyButton{{RETROK_F5, "F5"}, {X_F5, Y_0}},
    KeyButton{{RETROK_F6, "F6"}, {X_F6, Y_0}},
    KeyButton{{RETROK_F7, "F7"}, {X_F7, Y_0}},
    KeyButton{{RETROK_F8, "F8"}, {X_F8, Y_0}},
    KeyButton{{RETROK_F9, "F9"}, {X_F9, Y_0}},
    KeyButton{{RETROK_F10, "F10"}, {X_F10, Y_0}},
    KeyButton{{RETROK_F11, "F11"}, {X_F11, Y_0}},
    KeyButton{{RETROK_F12, "F12"}, {X_F12, Y_0}},
    KeyButton{{RETROK_PRINT, KEY_PRTSC}, {X_EXT0, Y_0}},
    KeyButton{{RETROK_SCROLLOCK, KEY_SCRLK, RETROKMOD_SCROLLOCK}, {X_EXT1, Y_0}},
    KeyButton{{RETROK_PAUSE, KEY_PAUSE}, {X_EXT2, Y_0}},

    KeyButton{{RETROK_BACKQUOTE, "`"}, {X_BACKQUOTE, Y_1}, {RETROK_TILDE, "~"}},
    KeyButton{{RETROK_1, "1"}, {X_1, Y_1}, {RETROK_EXCLAIM, "!"}},
    KeyButton{{RETROK_2, "2"}, {X_2, Y_1}, {RETROK_AT, "@"}},
    KeyButton{{RETROK_3, "3"}, {X_3, Y_1}, {RETROK_HASH, "#"}},
    KeyButton{{RETROK_4, "4"}, {X_4, Y_1}, {RETROK_DOLLAR, "$"}},
    KeyButton{{RETROK_5, "5"}, {X_5, Y_1}, {(retro_key)'%', "%"}},
    KeyButton{{RETROK_6, "6"}, {X_6, Y_1}, {RETROK_CARET, "^"}},
    KeyButton{{RETROK_7, "7"}, {X_7, Y_1}, {RETROK_AMPERSAND, "&"}},
    KeyButton{{RETROK_8, "8"}, {X_8, Y_1}, {RETROK_ASTERISK, "*"}},
    KeyButton{{RETROK_9, "9"}, {X_9, Y_1}, {RETROK_LEFTPAREN, "("}},
    KeyButton{{RETROK_0, "0"}, {X_0, Y_1}, {RETROK_RIGHTPAREN, ")"}},
    KeyButton{{RETROK_MINUS, "-"}, {X_MINUS, Y_1}, {RETROK_UNDERSCORE, "_"}},
    KeyButton{{RETROK_EQUALS, "="}, {X_EQUALS, Y_1}, {RETROK_PLUS, "+"}},
    KeyButton{{RETROK_BACKSPACE, "Backspace"}, {X_BACKSPACE, Y_1}, EMPTY_KEY, {KEY_BACKSPACE_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_INSERT, KEY_INSERT}, {X_EXT0, Y_1}},
    KeyButton{{RETROK_HOME, KEY_HOME}, {X_EXT1, Y_1}},
    KeyButton{{RETROK_PAGEUP, KEY_PAGEUP}, {X_EXT2, Y_1}},

    KeyButton{{RETROK_TAB, "Tab"}, {X_TAB, Y_2}, EMPTY_KEY, {KEY_TAB_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_q, "Q"}, {X_Q, Y_2}},
    KeyButton{{RETROK_w, "W"}, {X_W, Y_2}},
    KeyButton{{RETROK_e, "E"}, {X_E, Y_2}},
    KeyButton{{RETROK_r, "R"}, {X_R, Y_2}},
    KeyButton{{RETROK_t, "T"}, {X_T, Y_2}},
    KeyButton{{RETROK_y, "Y"}, {X_Y, Y_2}},
    KeyButton{{RETROK_u, "U"}, {X_U, Y_2}},
    KeyButton{{RETROK_i, "I"}, {X_I, Y_2}},
    KeyButton{{RETROK_o, "O"}, {X_O, Y_2}},
    KeyButton{{RETROK_p, "P"}, {X_P, Y_2}},
    KeyButton{{RETROK_LEFTBRACKET, "["}, {X_LEFTBRACKET, Y_2}, {RETROK_LEFTBRACE, "{"}},
    KeyButton{{RETROK_RIGHTBRACKET, "]"}, {X_RIGHTBRACKET, Y_2}, {RETROK_RIGHTBRACE, "}"}},
    KeyButton{{RETROK_BACKSLASH, "\\"}, {X_BACKSLASH, Y_2}, {RETROK_BAR, "|"}, {KEY_BUTTON_WIDTH * 1.5 + KEY_BUTTON_INTERVAL, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_DELETE, KEY_DELETE}, {X_EXT0, Y_2}},
    KeyButton{{RETROK_END, KEY_END}, {X_EXT1, Y_2}},
    KeyButton{{RETROK_PAGEDOWN, KEY_PAGEDOWN}, {X_EXT2, Y_2}},

    KeyButton{{RETROK_CAPSLOCK, "Cpas", RETROKMOD_CAPSLOCK}, {X_CAPSLOCK, Y_3}, EMPTY_KEY, {KEY_CAPSLOCK_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_a, "A"}, {X_A, Y_3}},
    KeyButton{{RETROK_s, "S"}, {X_S, Y_3}},
    KeyButton{{RETROK_d, "D"}, {X_D, Y_3}},
    KeyButton{{RETROK_f, "F"}, {X_F, Y_3}},
    KeyButton{{RETROK_g, "G"}, {X_G, Y_3}},
    KeyButton{{RETROK_h, "H"}, {X_H, Y_3}},
    KeyButton{{RETROK_j, "J"}, {X_J, Y_3}},
    KeyButton{{RETROK_k, "K"}, {X_K, Y_3}},
    KeyButton{{RETROK_l, "L"}, {X_L, Y_3}},
    KeyButton{{RETROK_SEMICOLON, ";"}, {X_SEMICOLON, Y_3}, {RETROK_COLON, ":"}},
    KeyButton{{RETROK_QUOTE, "'"}, {X_QUOTE, Y_3}, {RETROK_QUOTEDBL, "\""}},
    KeyButton{{RETROK_RETURN, "Enter"}, {X_RETURN, Y_3}, EMPTY_KEY, {KEY_RETURN_WIDTH, KEY_BUTTON_HEIGHT}},

    KeyButton{{RETROK_LSHIFT, "Shift", RETROKMOD_SHIFT}, {X_LSHIFT, Y_4}, EMPTY_KEY, {KEY_LSHIFT_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_z, "Z"}, {X_Z, Y_4}},
    KeyButton{{RETROK_x, "X"}, {X_X, Y_4}},
    KeyButton{{RETROK_c, "C"}, {X_C, Y_4}},
    KeyButton{{RETROK_v, "V"}, {X_V, Y_4}},
    KeyButton{{RETROK_b, "B"}, {X_B, Y_4}},
    KeyButton{{RETROK_n, "N"}, {X_N, Y_4}},
    KeyButton{{RETROK_m, "M"}, {X_M, Y_4}},
    KeyButton{{RETROK_COMMA, ","}, {X_COMMA, Y_4}, {RETROK_LESS, "<"}},
    KeyButton{{RETROK_PERIOD, "."}, {X_PERIOD, Y_4}, {RETROK_GREATER, ">"}},
    KeyButton{{RETROK_SLASH, "/"}, {X_SLASH, Y_4}, {RETROK_QUESTION, "?"}},
    KeyButton{{RETROK_RSHIFT, "Shift ", RETROKMOD_SHIFT}, {X_RSHIFT, Y_4}, EMPTY_KEY, {KEY_RSHIFT_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_UP, KEY_UP}, {X_EXT1, Y_4}},

    KeyButton{{RETROK_LCTRL, "Ctrl", RETROKMOD_CTRL}, {X_LCTRL, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_LMETA, KEY_WIN, RETROKMOD_META}, {X_LMETA, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_LALT, "Alt", RETROKMOD_ALT}, {X_LALT, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_SPACE, ""}, {X_SPACE, Y_5}, EMPTY_KEY, {KEY_SPACE_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_RALT, "Alt ", RETROKMOD_ALT}, {X_RALT, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_RMETA, KEY_WIN " ", RETROKMOD_META}, {X_RMETA, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_RCTRL, "Ctrl ", RETROKMOD_CTRL}, {X_RCTRL, Y_5}, EMPTY_KEY, {KEY_BOTTOM_WIDTH, KEY_BUTTON_HEIGHT}},
    KeyButton{{RETROK_LEFT, KEY_LEFT}, {X_EXT0, Y_5}},
    KeyButton{{RETROK_DOWN, KEY_DOWN}, {X_EXT1, Y_5}},
    KeyButton{{RETROK_RIGHT, KEY_RIGHT}, {X_EXT2, Y_5}},
};

Keyboard::Keyboard()
    : Locker("keyboard_mutex"),
      _visable(false),
      _mod(0),
      _callback(nullptr),
      _status{0}
{
    SetKeyboardDown();
};

Keyboard::~Keyboard() {};

void Keyboard::Show()
{
    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    ImGui::SetNextWindowPos(_pos);
    ImGui::SetNextWindowSize({KEYBOARD_WIDTH, KEYBOARD_HEIGHT});
    ImGui::SetNextWindowBgAlpha(0.5);

    if (ImGui::Begin("keyboard", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        bool shift_mod = _mod & RETROKMOD_SHIFT;
        for (const auto button : _buttons)
        {
            ImGui::SetCursorPos(button.pos);
            const Key *key;
            if (shift_mod && button.shift.key != RETROK_UNKNOWN)
            {
                key = &button.shift;
            }
            else
            {
                key = &button.key;
            }
            bool push_color = (key->mod & _mod) && _status[key->key];
            if (push_color)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_GREEN);
            }

            if (ImGui::Button(key->str, button.size))
            {
                _OnKeyUp(button.key); // release
            }
            else if (ImGui::IsItemActive())
            {
                _OnKeyDown(button.key); // hold
            }

            if (push_color)
            {
                ImGui::PopStyleColor();
            }
        }
    }
    ImGui::End();

    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
}

extern int mx, my;

void Keyboard::SetVisable(bool visable)
{
    LogFunctionName;
    LogDebug("  %d", visable);
    _visable = visable;
    ImGui_ImplVita2D_TouchUsage(visable);
    ImGui_ImplVita2D_GamepadUsage(visable);
    if (!visable)
    {
        ImGui::GetIO().MousePos = {-FLT_MAX, -FLT_MAX};
        mx = my = 0;
    }
};

void Keyboard::_OnKeyDown(const Key &key)
{
    LogFunctionName;
    LogDebug("  %04x %s", key.key, key.str);

    bool need_callback = false;

    Lock();
    if (!_status[key.key] && key.mod == RETROKMOD_NONE)
    {
        need_callback = _status[key.key] = true;
    }
    Unlock();

    if (need_callback && _callback)
    {
        LogDebug("  _callback down %04x %s %04x", key.key, key.str, _mod);
        _ModKeyDown();
        _callback(true, key.key, 0, _mod);
    }
}

void Keyboard::_OnKeyUp(const Key &key)
{
    LogFunctionName;
    LogDebug("  %04x %s", key.key, key.str);

    bool down;

    Lock();
    if (key.mod == RETROKMOD_NONE) // normal keys
    {
        down = _status[key.key] = false;
    }
    else // shift, ctrl, alt, win, capslock keys
    {
        down = _status[key.key] = !_status[key.key];
        _mod ^= key.mod;
    }
    Unlock();

    if (_callback)
    {
        LogDebug("  _callback %d %04x %s %04x", down, key.key, key.str, _mod);
        _callback(down, key.key, 0, _mod);
    }
}

bool Keyboard::CheckKey(retro_key key)
{
    Lock();
    bool result = _status[key];
    Unlock();

    // LogDebug("CheckKey %04x %d", key, result);
    return result;
}

void Keyboard::_ModKeyDown()
{
    //_callback should be checked when call
    for (const auto &key : {RETROK_RSHIFT,
                            RETROK_LSHIFT,
                            RETROK_RCTRL,
                            RETROK_LCTRL,
                            RETROK_RALT,
                            RETROK_LALT,
                            RETROK_LSUPER,
                            RETROK_RSUPER})
    {
        if (_status[key])
        {
            _callback(true, key, 0, _mod);
        }
    }
}
