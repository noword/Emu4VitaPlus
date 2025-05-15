#pragma once
#include <vector>
#include <libretro.h>
#include <psp2common/ctrl.h>
#include "config_types.h"
#include "input.h"

#define ROOT_DIR "ux0:data/EMU4VITAPLUS"
#define PLAYLISTS_DIR ROOT_DIR "/playlists"
#define THUMBNAILS_DIR ROOT_DIR "/thumbnails"
#define CACHE_DIR ROOT_DIR "/cache"
#define CORE_SYSTEM_DIR ROOT_DIR "/system"
#define ARCHIVE_CACHE_DIR CACHE_DIR "/archives"
#define ARCADE_CACHE_DIR CACHE_DIR "/arcades"
#define PLAYLISTS_CACHE_DIR CACHE_DIR "/playlists"
#define OVERLAYS_DIR_NAME "overlays"
#define PREVIEW_DIR_NAME ".previews"
#define CHEAT_DIR_NAME ".cheats"

extern const char CONSOLE_DIR[];

extern const char CORE_SHORT_NAME[];
extern const char CORE_DATA_DIR[];
extern const char CORE_SAVEFILES_DIR[];
extern const char CORE_CHEATS_DIR[];
extern const char CORE_LOG_PATH[];
extern const char CORE_CONFIG_PATH[];
extern const char CORE_INPUT_DESC_PATH[];
extern const char CORE_FAVOURITE_PATH[];

extern const bool DEFAULT_ENABLE_REWIND;
extern const size_t DEFAULT_REWIND_BUF_SIZE;
extern const bool DEFAULT_AUTO_SAVE;
extern const bool DEFAULT_REBOOT_WHEN_LOADING_AGAIN;

extern const bool DEFAULT_INDEPENDENT_CONFIG;

extern const bool DEFAULT_LIGHTGUN;
extern const bool DEFAULT_MOUSE;
extern const bool ENABLE_KEYBOARD;

extern const bool CONTROL_SPEED_BY_VIDEO;

extern const std::vector<uint8_t> RETRO_KEYS;
extern const std::vector<ControlMapConfig> CONTROL_MAPS;
extern const std::vector<std::pair<const char *, const char *>> DEFAULT_CORE_SETTINGS;

struct BIOS
{
    const char *filename;
    uint32_t crc32 = 0;
};

extern const std::vector<BIOS> REQUIRED_BIOS;