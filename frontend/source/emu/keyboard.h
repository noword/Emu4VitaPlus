#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <libretro.h>
#include <string>
#include <bitset>
#include <stdint.h>
#include "input.h"
#include "locker.h"

namespace Emu4VitaPlus
{

#define KEYBOARD_WIDTH (KEY_BUTTON_NEXT * 18.5)
#define KEYBOARD_HEIGHT (KEY_BUTTON_NEXT * 6)

#define KEY_BUTTON_WIDTH 50
#define KEY_BUTTON_HEIGHT 50
#define KEY_BUTTON_INTERVAL 2
#define KEY_BUTTON_NEXT (KEY_BUTTON_WIDTH + KEY_BUTTON_INTERVAL)
#define EMPTY_KEY {RETROK_UNKNOWN, nullptr}

    struct Key
    {
        unsigned key;
        const char *str;
        uint16_t mod = RETROKMOD_NONE;
    };

    struct KeyButton
    {
        Key key;
        ImVec2 pos;
        Key shift = EMPTY_KEY;
        ImVec2 size = {KEY_BUTTON_WIDTH, KEY_BUTTON_HEIGHT};
    };

    class Keyboard : public Locker
    {
    public:
        Keyboard();
        virtual ~Keyboard();

        void Show();
        void SetInputHooks(Input *input);
        void UnsetInputHooks(Input *input);

        bool Visable() { return _visable; };
        void SetVisable(bool visable);
        bool CheckKey(retro_key key);
        void SetCallback(retro_keyboard_event_t callback) { _callback = callback; };
        void SetKeyboardUp() { _pos = {.0, .0}; };
        void SetKeyboardDown() { _pos = {(VITA_WIDTH - KEYBOARD_WIDTH) / 2, VITA_HEIGHT - KEYBOARD_HEIGHT}; };

    private:
        void _OnKeyDown(const Key &key);
        void _OnKeyUp(const Key &key);
        void _ModKeyDown();

        ImVec2 _pos;
        static const KeyButton _buttons[];
        std::bitset<RETROK_LAST> _status;
        uint16_t _mod;
        bool _visable;

        retro_keyboard_event_t _callback;
    };

};