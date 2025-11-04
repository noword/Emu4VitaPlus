#pragma once
#include <string>
#include <stdint.h>
#include <vita2d.h>
#include <psp2/kernel/threadmgr/thread.h>
#include <map>
#include <locker.h>

// 5 seconds
#define NOTIFICATION_DISPLAY_TIME 5000000

struct Notification
{
    Notification() : texture(nullptr), _stop_time(0) {};
    virtual ~Notification() {};

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

class Notifications
{
public:
    Notifications();
    virtual ~Notifications();

    bool NeedShow() { return !_notifications.empty(); };
    void Show();
    void Run();
    void Add(uint32_t id, Notification *n);
    void Remove(uint32_t id);
    void Update(uint32_t id, const std::string &title, const std::string &text = "", vita2d_texture *texture = nullptr);
    void Clear();

private:
    std::map<uint32_t, Notification *> _notifications;
    Locker _locker;
};