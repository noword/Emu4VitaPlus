#pragma once
#include <vita2d.h>
#include <imgui_vita2d/imgui_vita.h>

class TimeScale
{
public:
    TimeScale();
    virtual ~TimeScale();

    void Show();
    void SetTime(int year, int offset);

private:
    char _year[8];
    float _year_offset;
    float _x;
    vita2d_texture *_scale_texture;
    float _width;
};