#pragma once

#define APP_ASSETS_DIR "app0:assets"
#define CORE_DATA_DIR "app0:data"
#define APP_DATA_DIR "ux0:data/EMU4VITAPLUS/" APP_DIR_NAME
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
    PCE,
    AMIGA,
    X68000,
    MD,
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