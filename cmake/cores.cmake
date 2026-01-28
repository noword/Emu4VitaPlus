cmake_minimum_required(VERSION 3.12)

# CORE_DIR;CORE;CORE_SHORT;CONSOLE;TITLE_ID;
set(CORE_ROWS
    "gpsp;gpsp;gpSP;GBA;GPSP4VITA"
    "vba-next;vba_next;VBANext;GBA;VBANEXT4V"
    "gambatte-libretro;gambatte;Gambatte;GBC;GAMBATTEV"
    "libretro-fba-lite;fba_lite;FBALite;ARC;FBALITE4V"
    "libretro-fbneo;fbneo;FBNeo;ARC;FBNEO4VIT"
    "fbalpha2012;fbalpha2012;FBA2012;ARC;FBA20124V"
    "snes9x2002;snes9x2002;Snes9x2002;SNES;SNES024VT"
    "snes9x2005;snes9x2005_plus;Snes9x2005Plus;SNES;SNES054VT"
    "libretro-fceumm;fceumm;FCEUmm;NES;FCEUMM4VT"
    "nestopia;nestopia;Nestopia;NES;NESTP4VIT"
    "Genesis-Plus-GX;genesis_plus_gx;GenesisPlusGX;MD;GENESIS4V"
    "Genesis-Plus-GX-Wide;genesis_plus_gx_wide;GenesisPlusGXWide;MD;GENWIDE4V"
    "picodrive;picodrive;PicoDrive;MD;PICODR4VT"
    "beetle-pce-fast-libretro;mednafen_pce_fast;MednafenPCEFast;PCE;MPCEF4VIT"
    "beetle-supergrafx-libretro;mednafen_supergrafx;MednafenPCESuperGrafx;PCE;MPCESG4VT"
    "beetle-ngp-libretro;mednafen_ngp;MednafenNgp;NGP;MNGP4VITA"
    "beetle-wswan-libretro;mednafen_wswan;MednafenWswan;WSC;WSWAN4VIT"
    "pcsx_rearmed;pcsx_rearmed;PCSXReARMed;PS;PCSX4V001"
    "stella2014-libretro;stella2014;Stella2014;ATARI2600;STELLA4VI"
    "prosystem-libretro;prosystem;ProSystem;ATARI7800;PROSYS4PV"
    "libretro-atari800;atari800;Atari800;ATARI5200;A8004VITA"
    "dosbox-pure;dosbox_pure;DOSBoxPure;DOS;DOSPURE4V"
    "mame2000-libretro;mame2000;MAME2000;ARC;MAME2K4VI"
    "mame2003-plus-libretro;mame2003_plus;MAME2003Plus;ARC;MAME03P4V"
    "mame2003-libretro;mame2003;MAME2003;ARC;MAME2K34V"
    "libretro-vecx;vecx;vecx;VECTREX;VECX4VITA"
    "uae4arm-libretro;uae4arm;uae4arm;AMIGA;UAE4VITAP"
    "fuse-libretro;fuse;fuse;ZXS;FUSE4VITA"
    "neocd_libretro;neocd;neocd;NEOGEOCD;NEOCD4VIT"
    "supafaust;mednafen_supafaust;Supafaust;SNES;SUPA4VITA"
    "FBNeo-Xtreme-Amped;km_fbneo_xtreme_amped;FBNeoXtreme;ARC;FBNEOX4VP"
    "mame2003-xtreme;km_mame2003_xtreme_amped;MAME2003Xtreme;ARC;MAME2K3XV"
    "chimerasnes;chimerasnes;ChimeraSNES;SNES;CHIMERA4V"
    "tgbdual-libretro;tgbdual;TGBDual;GBC;TGBD4VITA"
    "libretro-meowPC98;nekop2;nekop2;PC98;NEKOP2VIT"
    "fmsx-libretro;fmsx;fMSX;MSX;FMSX4VITA"
    "blueMSX-libretro;bluemsx;blueMSX;MSX;BLUEMSX4V"
    "NP2kai;np2kai;np2kai;PC98;NP2KAIVIT"
    "vice-libretro;vice;vice;C64;VICE4VITA"
    "px68k-libretro;px68k;px68k;X68000;PX68K4VIT"
    "beetle-lynx-libretro;mednafen_lynx;Lynx;LYNX;LYNX4VITA"
    "libretro-handy;handy;Handy;LYNX;HANDY4VIT"
    "libretro-cap32;cap32;Caprice32;CPC;CAP324VIT"
    "libretro-crocods;crocods;CrocoDS;CPC;CROCODS4V"
    "snes9x;snes9x;Snes9x;SNES;SNES9X4VT"
    "mgba;mgba;mGBA;GBA;MGBA4VITA"
    "parallel-n64;parallel_n64;mupen64plus;N64;PN644VITA"
)

set(ROW_SIZE 5)

macro(GET_CORES)
    set(CORE_DIRS "" CACHE INTERNAL "CORE_DIRS")
    set(CORES "" CACHE INTERNAL "CORES")
    set(CORE_SHORTS "" CACHE INTERNAL "CORE_SHORTS")
    set(CONSOLES "" CACHE INTERNAL "CONSOLES")
    set(TITLE_IDS "" CACHE INTERNAL "TITLE_IDS")

    list(LENGTH CORE_ROWS length)
    math(EXPR length "${length} / ${ROW_SIZE} - 1")

    foreach(ROW RANGE ${length})
        math(EXPR INDEX "${ROW} * ${ROW_SIZE}")

        list(GET CORE_ROWS ${INDEX} V)
        list(APPEND CORE_DIRS ${V})

        math(EXPR INDEX "${INDEX} + 1")
        list(GET CORE_ROWS ${INDEX} V)
        list(APPEND CORES ${V})

        math(EXPR INDEX "${INDEX} + 1")
        list(GET CORE_ROWS ${INDEX} V)
        list(APPEND CORE_SHORTS ${V})

        math(EXPR INDEX "${INDEX} + 1")
        list(GET CORE_ROWS ${INDEX} V)
        list(APPEND CONSOLES ${V})

        math(EXPR INDEX "${INDEX} + 1")
        list(GET CORE_ROWS ${INDEX} V)
        list(APPEND TITLE_IDS ${V})
    endforeach()
endmacro()