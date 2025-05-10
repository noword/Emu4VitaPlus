#pragma once

enum CONSOLE
{
    ATARI2600 = 1,
    ATARI5200 = 1 << 1,
    ATARI7800 = 1 << 2,
    C64 = 1 << 3,
    VECTREX = 1 << 4,
    ZXS = 1 << 5,
    DOS = 1 << 6,
    PC98 = 1 << 7,
    MSX = 1 << 8,
    NES = 1 << 9,
    AMIGA = 1 << 10,
    X68000 = 1 << 11,
    PCE = 1 << 12,
    MD = 1 << 13,
    GBC = 1 << 14,
    SNES = 1 << 15,
    NEOGEOCD = 1 << 16,
    PS1 = 1 << 17,
    NGP = 1 << 18,
    WSC = 1 << 19,
    GBA = 1 << 20,
    ARC = 1 << 21,
    CONSOLE_LAST,
    CONSOLE_COUNT = CONSOLE_LAST - 1,
};

enum CORE
{
    GPSP = 0,
    VBA_NEXT,
    GAMBATTE,
    FBA_LITE,
    FBNEO,
    FBALPHA2012,
    SNES9X2002,
    SNES9X2005,
    SNES9X2010,
    FCEUMM,
    NESTOPIA,
    GENESIS_PLUS_GX,
    GENESIS_PLUS_GX_WIDE,
    PICODRIVE,
    MEDNAFEN_PCE_FAST,
    MEDNAFEN_SUPERGRAFX,
    MEDNAFEN_NGP,
    MEDNAFEN_WSWAN,
    PCSX_REARMED,
    STELLA2014,
    PROSYSTEM,
    ATARI800,
    DOSBOX_PURE,
    MAME2003_PLUS,
    MAME2003,
    VECX,
    UAE4ARM,
    FUSE,
    NEOCD,
    SUPAFAUST,
    FBNEO_XTREME,
    MAME2003_XTREME,
    CHIMERASNES,
    TGBDUAL,
    NEKOP2,
    FMSX,
    BLUEMSX,
    NEKOP2KAI,
    VICE,
    PX68K,
    CORE_COUNT
};

struct CORE_STRUCT
{
    const char *core_name;
    const char *console_name;
    CORE core;
    CONSOLE console;
};

extern const CORE_STRUCT CORES[CORE_COUNT];
