#pragma once

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
