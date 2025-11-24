#pragma once
#include "video_thread.h"
#include "log.h"

extern Emu4VitaPlus::VideoThread *gVideo;

enum APP_STATUS
{
    APP_STATUS_BOOT = 1,
    APP_STATUS_SHOW_UI = 1 << 2,
    APP_STATUS_RUN_GAME = 1 << 3,
    APP_STATUS_SHOW_UI_IN_GAME = 1 << 4,
    APP_STATUS_REWIND_GAME = 1 << 5,
    APP_STATUS_RETURN_ARCH = 1 << 6,
    APP_STATUS_REBOOT_WITH_LOADING = 1 << 7,
    APP_STATUS_EXIT = 1 << 8,
};

class AppStatus
{
public:
    AppStatus(APP_STATUS status = APP_STATUS_BOOT) : _status(status) {};
    virtual ~AppStatus() {};

    APP_STATUS Get() { return _status; };
    void Set(APP_STATUS status)
    {
        if (_status != status)
        {
            LogInfo("Status from %d to %d", _status, status);
            gVideo->Lock();
            _status = status;
            gVideo->Unlock();
        }
    };

private:
    APP_STATUS _status;
};