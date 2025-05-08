#pragma once
#include <vector>
#include <stdint.h>
#include "config_types.h"

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

#define DEFINE_STRING_LENGTH 0x20
#define DEFINE_PATH_LENGTH 0x40

extern char CORE_NAME[DEFINE_STRING_LENGTH];
extern char CORE_SHORT_NAME[DEFINE_STRING_LENGTH];
extern char CORE_DATA_DIR[DEFINE_PATH_LENGTH];
extern char CORE_SAVEFILES_DIR[DEFINE_PATH_LENGTH];
extern char CORE_CHEATS_DIR[DEFINE_PATH_LENGTH];
extern char CORE_LOG_PATH[DEFINE_PATH_LENGTH];
extern char CORE_CONFIG_PATH[DEFINE_PATH_LENGTH];
extern char CORE_INPUT_DESC_PATH[DEFINE_PATH_LENGTH];
extern char CORE_FAVOURITE_PATH[DEFINE_PATH_LENGTH];

extern char CONSOLE_DIR[DEFINE_PATH_LENGTH];
// extern const char APP_DATA_DIR[];
// extern const char APP_LOG_PATH[];
// extern const char APP_CONFIG_PATH[];
// extern const char APP_INPUT_DESC_PATH[];
// extern const char APP_FAVOURITE_PATH[];
// extern const char CORE_SAVEFILES_DIR[];
// extern const char CORE_CHEATS_DIR[];
// extern const char APP_TITLE_NAME[];
// extern const char CORE_FULL_NAME[];
// extern const char APP_VER_STR[];
// extern const char CONSOLE[];

extern const uint8_t RETRO_KEYS[];
extern const size_t RETRO_KEYS_SIZE;
extern const std::vector<ControlMapConfig> CONTROL_MAPS;

extern const bool DEFAULT_ENABLE_REWIND;
extern const size_t DEFAULT_REWIND_BUF_SIZE;
extern const bool DEFAULT_FRONT_TOUCH;
extern const bool DEFAULT_REAR_TOUCH;
extern const bool DEFAULT_MOUSE;
extern const bool DEFAULT_LIGHTGUN;
extern const bool DEFAULT_AUTO_SAVE;
extern const bool DEFAULT_INDEPENDENT_CORE_CONFIG;
extern const bool DEFAULT_REBOOT_WHEN_LOADING_AGAIN;
extern const bool CONTROL_SPEED_BY_VIDEO;
extern const bool ENABLE_KEYBOARD;

extern const bool IS_ARCADE;