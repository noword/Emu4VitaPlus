#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <libretro.h>
#include <string>
#include <vector>
#include <stdint.h>
#include "input.h"

namespace Emu4VitaPlus
{
#define KEY_BUTTON_WIDTH 50
#define KEY_BUTTON_HEIGHT 50
#define KEY_BUTTON_INTERVAL 2
#define KEY_BUTTON_NEXT (KEY_BUTTON_WIDTH + KEY_BUTTON_INTERVAL)
#define EMPTY_KEY {RETROK_UNKNOWN, nullptr}

    struct Key
    {
        retro_key key;
        const char *str;
        retro_mod mod = RETROKMOD_NONE;
        bool down = false;
        bool last_down = false;
    };

    struct KeyButton
    {
        Key key;
        ImVec2 pos;
        Key shift = EMPTY_KEY;
        ImVec2 size = {KEY_BUTTON_WIDTH, KEY_BUTTON_HEIGHT};
    };

    class Keyboard
    {
    public:
        Keyboard();
        virtual ~Keyboard();

        void Show();
        void SetInputHooks(Input *input);
        void UnsetInputHooks(Input *input);

        bool Visable() { return _visable; };
        void SetVisable(bool visable) { _visable = visable; };

    private:
        std::vector<KeyButton> _buttons;
        retro_key _down_key;
        uint8_t _mod;
        bool _visable;
    };
};