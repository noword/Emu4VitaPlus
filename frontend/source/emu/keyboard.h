#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <psp2/kernel/threadmgr.h>
#include <libretro.h>
#include <string>
#include <bitset>
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

    class Keyboard
    {
    public:
        Keyboard();
        virtual ~Keyboard();

        void Show();
        void SetInputHooks(Input *input);
        void UnsetInputHooks(Input *input);

        bool Visable() { return _visable; };
        void SetVisable(bool visable);
        int32_t Lock(uint32_t *timeout = NULL);
        void Unlock();
        bool CheckKey(retro_key key);
        void SetCallback(retro_keyboard_event_t callback) { _callback = callback; };

    private:
        void _OnKeyDown(const Key &key);
        void _OnKeyUp(const Key &key);

        static const KeyButton _buttons[];
        std::bitset<RETROK_LAST> _status;
        uint16_t _mod;
        bool _visable;
        SceKernelLwMutexWork _mutex;

        retro_keyboard_event_t _callback;
    };

};