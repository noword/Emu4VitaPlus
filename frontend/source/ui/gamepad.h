#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <vector>
#include "input.h"

namespace Emu4VitaPlus
{
    struct Button
    {
        const char *text;
        ImVec2 pos;
    };

    struct Point : public TouchPoint
    {
        float radius = 0.f;
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
        void _UpdatePoints(std::vector<Point> *points, const TouchState *states);

        bool _actived;
        static Button _buttons[14];
        bool _start_pressed;
        uint64_t _close_time;
        Input *_input;
        std::vector<Point> _front;
        std::vector<Point> _rear;
    };
};