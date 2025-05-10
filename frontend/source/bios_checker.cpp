#include <stdint.h>
#include "bios_checker.h"
#include "log.h"
#include "file.h"
#include "global.h"

bool CheckBios(std::vector<const char *> &missing_bios_files)
{
    bool result = true;
    char tmp[SCE_FIOS_PATH_MAX];
    char *buf;

    for (auto const &bios : REQUIRED_BIOS)
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