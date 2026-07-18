#pragma once
#include <vita2d.h>
#include <imgui_vita2d/imgui_vita.h>
#include "defines.h"

class TimeScale
{
public:
    TimeScale();
    virtual ~TimeScale();

    void Show();
    void SetTime(int year, int offset);

private:
    char _year[8];
    vita2d_texture *_scale_texture;
    float _width;
    float _x[GRADIENT_FRAMES];
    float _year_x[GRADIENT_FRAMES];
    int _count;
};