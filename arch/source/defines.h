#pragma once

#define APP_ASSETS_DIR "app0:assets"
#define CORE_DATA_DIR "app0:data"
#define ROOT_DIR "ux0:data/EMU4VITAPLUS"
#define APP_DATA_DIR ROOT_DIR "/" APP_DIR_NAME
#define CACHE_DIR ROOT_DIR "/cache"
#define APP_LOG_PATH APP_DATA_DIR "/Emu4Vita++.log"
#define APP_CONFIG_PATH APP_DATA_DIR "/config.ini"

#define BUTTON_SIZE 158
#define MAIN_WINDOW_PADDING 10
#define ROW_COUNT 2

enum CONSOLE
{
    ATARI2600 = 0,
    ATARI5200,
    ATARI7800,
    C64,
    VECTREX,
    ZXS,
    DOS,
    PC98,
    MSX,
    NES,
    CPC,
    AMIGA,
    X68000,
    PCE,
    MD,
    LYNX,
    GBC,
    SNES,
    NEOCD,
    PS1,
    NGP,
    WSC,
    GBA,
    ARC,
    CONSOLE_COUNT
};

extern const char *CONSOLE_NAMES[];