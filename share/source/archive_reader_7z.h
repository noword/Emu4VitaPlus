#pragma once
#include <7z.h>
#include <7zFile.h>
#include <7zAlloc.h>
#include <7zCrc.h>
#include <stdint.h>
#include "archive_reader.h"

class ArchiveReader7z : public ArchiveReader
{
public:
    ArchiveReader7z();
    virtual ~ArchiveReader7z();

    bool Open(const char *path);
    void Close();
    bool First();
    bool Next();
    bool ExtractCurrent(const char *dst_path);
    const char *GetCurrentName();
    uint32_t GetCurrentCrc32();
    size_t GetCurrentSize();

private:
    static bool _init_crc_table;

    CSzArEx _db;
    CFileInStream _archive_stream;
    CLookToRead2 _look_stream;
    static const ISzAlloc _alloc_imp;
    uint8_t *_buf;
    size_t _index;
};