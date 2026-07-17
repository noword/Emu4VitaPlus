#pragma once
#include <vita2d.h>
#include <imgui_vita2d/imgui_vita.h>

#define STEPS 30

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
    float _x[STEPS];
    float _year_x[STEPS];
    int _count;
};