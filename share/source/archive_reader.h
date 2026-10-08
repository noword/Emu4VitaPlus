#pragma once
#include <stdint.h>
#include "file.h"

// libarchive is too bloated and does not support crc32

class ArchiveReader
{
public:
    ArchiveReader() = default;
    virtual ~ArchiveReader() = default;

    virtual bool Open(const char *path) = 0;
    virtual void Close() = 0;
    virtual bool First() = 0;
    virtual bool Next() = 0;
    virtual bool ExtractCurrent(const char *dst_path) = 0;
    virtual const char *GetCurrentName() = 0;
    virtual uint32_t GetCurrentCrc32() = 0;
    virtual size_t GetCurrentSize() = 0;

    bool ExtractByHash(uint32_t crc32, const char *dst_path)
    {
        if (!First())
        {
            return false;
        }

        bool result = false;
        do
        {
            if (crc32 == GetCurrentCrc32())
            {
                result = ExtractCurrent(dst_path);
                break;
            }
        } while (Next());
        return result;
    }
};
