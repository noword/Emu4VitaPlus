#include <stdint.h>
#include "bios_checker.h"
#include "log.h"
#include "file.h"
#include "defines.h"

struct BIOS
{
    const char *filename;
    uint32_t crc32 = 0;
};

BIOS RequiredBIOS[] = {
#if defined(ATARI2600_BUILD)
#elif defined(ATARI5200_BUILD)
    {"5200.rom", 0x4248d3e3},
    {"ATARIXL.ROM", 0x1f9cd270},
    {"ATARIBAS.ROM", 0x7d684184},
    {"ATARIOSA.ROM", 0x72b3fed4},
    {"ATARIOSB.ROM", 0x3e28a1fe},
#elif defined(ATARI7800_BUILD)
    {"7800 BIOS (U).rom", 0x5d13730c},
#elif defined(FUSE_BUILD)
    {"fuse/128p-0.rom", 0},
    {"fuse/128p-1.rom", 0},
    {"fuse/trdos.rom", 0},
    {"fuse/gluck.rom", 0},
    {"fuse/256s-0.rom", 0},
    {"fuse/256s-1.rom", 0},
    {"fuse/256s-2.rom", 0},
    {"fuse/256s-3.rom", 0},
#elif defined(MSX_BUILD)
    {"MSX.ROM", 0},
    {"MSX2.ROM", 0},
    {"MSX2EXT.ROM", 0},
    {"MSX2P.ROM", 0},
    {"MSX2PEXT.ROM", 0},
    {"DISK.ROM", 0},
    {"FMPAC.ROM", 0},
    {"MSXDOS2.ROM", 0},
    {"PAINTER.ROM", 0},
    {"KANJI.ROM", 0},
#elif defined(FC_BUILD)
    {"disksys.rom", 0x5e607dcf},
#elif defined(SFC_BUILD)
#elif defined(MD_BUILD)
#elif defined(NGP_BUILD)
#elif defined(PCE_BUILD)
    {"syscard3.pce", 0},
#elif defined(TGBDUAL_BUILD)
#elif defined(GAMEBATTE_BUILD)
    {"gb_bios.bin", 0},
    {"gbc_bios.bin", 0},
#elif defined(GBA_BUILD)
    {"gba_bios.bin", 0},
#if defined(MGBA_BUILD)
    {"gba_bios.bin", 0},
    {"gb_bios.bin", 0},
    {"gbc_bios.bin", 0},
    {"sgb_bios.bin", 0},
#endif
#elif defined(NEOCD_BUILD)
    {"neocd/neocd_f.rom", 0},
    {"neocd/neocd_sf.rom", 0},
    {"neocd/neocd_t.rom", 0},
    {"neocd/neocd_st.rom", 0},
    {"neocd/neocd_z.rom", 0},
    {"neocd/neocd_sz.rom", 0},
    {"neocd/front-sp1.bin", 0},
    {"neocd/top-sp1.bin", 0},
    {"neocd/neocd.bin", 0},
    {"neocd/uni-bioscd.rom", 0},
#elif defined(MAME2000_BUILD) || defined(MAME2003_BUILD) || defined(MAME2003PLUS_BUILD) || defined(MAME2000XTREME_BUILD)
#elif defined(ARC_BUILD)
#elif defined(DOS_BUILD)
#elif defined(AMIGA_BUILD)
#elif defined(VECTREX_BUILD)
#elif defined(PS_BUILD)
    {"PSXONPSP660.bin", 0},
    {"scph101.bin", 0},
    {"scph7001.bin", 0},
    {"scph5501.bin", 0},
    {"scph1001.bin", 0},
#elif defined(PC98_BUILD)
    {"np2/font.bmp", 0},
    {"np2/FONT.ROM", 0},
    {"np2/bios.rom", 0},
    {"np2/itf.rom", 0},
    {"np2/sound.rom", 0},
    {"np2/bios9821.rom", 0},
    {"np2/d8000.rom", 0},
    {"np2/2608_BD.WAV", 0},
    {"np2/2608_SD.WAV", 0},
    {"np2/2608_TOP.WAV", 0},
    {"np2/2608_HH.WAV", 0},
    {"np2/2608_TOM.WAV", 0},
    {"np2/2608_RIM.WAV", 0},
#else
#error "unknown build"
#endif

};

bool CheckBios(std::vector<const char *> &missing_bios_files)
{
    bool result = true;
    char tmp[SCE_FIOS_PATH_MAX];
    char *buf;

    for (auto const &bios : RequiredBIOS)
    {
        snprintf(tmp, SCE_FIOS_PATH_MAX, CORE_SYSTEM_DIR "/%s", bios.filename);
        if (File::Exist(tmp))
        {
            if (bios.crc32)
            {
                uint32_t crc32 = File::GetCrc32(tmp);
                if (crc32 == 0)
                {
                    missing_bios_files.push_back(bios.filename);
                    LogError("failed to get CRC32 of bios file: %s", tmp);
                    result = false;
                }
                else if (crc32 != bios.crc32)
                {
                    missing_bios_files.push_back(bios.filename);
                    LogError("incorrect CRC32 of BIOS file: %s(%08x), it should be %08x", tmp, crc32, bios.crc32);
                    result = false;
                }
                else
                {
                    LogDebug("BIOS file exists: %s", tmp);
                }
            }
        }
        else
        {
            missing_bios_files.push_back(bios.filename);
            LogError("BIOS file does not exist: %s", tmp);
            result = false;
        }
    }
    return result;
}