#pragma once
#include <stdint.h>
#include "config_types.h"
#include "video.h"
#include "app_status.h"
#include "overlay.h"
#include "log.h"
#include "ra_lpl.h"
#include "ui.h"
#include "emulator.h"
#include "config.h"
#include "favorite.h"
#include "state_manager.h"
#include "shader.h"
#include "rom_name.h"

extern bool gBootFromArch;
extern std::string gBootRomPath;

extern AppStatus gStatus;
extern Emu4VitaPlus::Video *gVideo;
extern Overlays *gOverlays;
extern RetroArchPlaylists *gPlaylists;
extern Ui *gUi;
extern Emulator *gEmulator;
extern Emu4VitaPlus::Config *gConfig;
extern Favorites *gFavorites;
extern CoreStateManager *gStateManager;
extern Shaders *gShaders;
extern RomNameMap *gRomNameMap;
