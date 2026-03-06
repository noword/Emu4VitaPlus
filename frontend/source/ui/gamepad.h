#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include "input.h"

namespace Emu4VitaPlus
{
    struct Button
    {
        const char *text;
        ImVec2 pos;
    };

    class Gamepad
    {
    public:
        Gamepad();
        virtual ~Gamepad();

        void Show();
        void SetActive(bool active) { _actived = active; };
        bool IsActive() { return _actived; };
        void SetInput(Input *input) { _input = input; };

    private:
        bool _actived;
        static Button _buttons[14];
        bool _start_pressed;
        uint64_t _close_time;
        Input *_input;
    };
};