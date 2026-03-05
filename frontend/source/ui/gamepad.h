#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include "input.h"

namespace Emu4VitaPlus
{
#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 50

    struct Button
    {
        const char *text;
        ImVec2 pos;
        ImVec2 size = {BUTTON_WIDTH, BUTTON_HEIGHT};
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