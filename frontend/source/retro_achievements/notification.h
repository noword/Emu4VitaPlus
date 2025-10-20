#pragma once
#include <string>
#include <stdint.h>
#include <vita2d.h>
#include <psp2/kernel/threadmgr/thread.h>

// 5 seconds
#define NOTIFICATION_DISPLAY_TIME 5000000

struct Notification
{
    Notification() : texture(nullptr), _stop_time(0) {};

    virtual ~Notification()
    {
        if (texture)
            vita2d_free_texture(texture);
    }

    bool TimeUp() const
    {
        return _stop_time != 0 && sceKernelGetSystemTimeWide() >= _stop_time;
    }

    void SetShowTime(uint64_t time = NOTIFICATION_DISPLAY_TIME)
    {
        _stop_time = sceKernelGetSystemTimeWide() + time;
    }

    std::string title;
    std::string text;
    vita2d_texture *texture;

private:
    uint64_t _stop_time;
};