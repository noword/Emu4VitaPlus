#pragma once
#include <string>
#include <vector>
#include <utility>
#include <stdint.h>
#include "config_types.h"
#include "defines.h"
#include "cores.h"
#include "video.h"
#include "log.h"

extern bool gBootFromArch;
extern std::string gBootRomPath;
extern const CORE_STRUCT *gCore;

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
    AppStatus() : _status(APP_STATUS_BOOT) {};
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

extern AppStatus gStatus;

extern char CONSOLE_DIR[DEFINE_PATH_LENGTH];
extern char CORE_DATA_DIR[DEFINE_PATH_LENGTH];
extern char CORE_SAVEFILES_DIR[DEFINE_PATH_LENGTH];
extern char CORE_CHEATS_DIR[DEFINE_PATH_LENGTH];
extern char CORE_LOG_PATH[DEFINE_PATH_LENGTH];
extern char CORE_CONFIG_PATH[DEFINE_PATH_LENGTH];
extern char CORE_INPUT_DESC_PATH[DEFINE_PATH_LENGTH];
extern char CORE_FAVOURITE_PATH[DEFINE_PATH_LENGTH];

extern bool DEFAULT_ENABLE_REWIND;
extern size_t DEFAULT_REWIND_BUF_SIZE;
extern bool DEFAULT_AUTO_SAVE;
extern bool DEFAULT_REBOOT_WHEN_LOADING_AGAIN;

extern bool DEFAULT_INDEPENDENT_CONFIG;

extern bool DEFAULT_LIGHTGUN;
extern bool DEFAULT_MOUSE;
extern bool ENABLE_KEYBOARD;

extern bool CONTROL_SPEED_BY_VIDEO;

extern std::vector<uint8_t> RETRO_KEYS;
extern std::vector<ControlMapConfig> CONTROL_MAPS;
extern std::vector<std::pair<const char *, const char *>> DEFAULT_CORE_SETTINGS;

struct BIOS
{
    const char *filename;
    uint32_t crc32 = 0;
};

extern std::vector<BIOS> REQUIRED_BIOS;

extern bool InitDefines();
