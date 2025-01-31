#include "archive_reader_7z.h"
#include "file.h"
#include "utils.h"
#include "log.h"

#define LOOK_STREAM_BUF_SIZE (1 << 18)

const ISzAlloc ArchiveReader7z::_alloc_imp{SzAlloc, SzFree};
bool ArchiveReader7z::_init_crc_table = false;

ArchiveReader7z::ArchiveReader7z()
{
    _buf = new uint8_t[LOOK_STREAM_BUF_SIZE];
    if (!_init_crc_table)
    {
        _init_crc_table = true;
        CrcGenerateTable();
    }
};

ArchiveReader7z::~ArchiveReader7z()
{
    delete[] _buf;
}

bool ArchiveReader7z::Open(const char *path)
{
    LogDebug("ArchiveReader7z::Open %s", path);
    if (InFile_Open(&_archive_stream.file, path) != 0)
    {
        LogError("failed to open %s", path);
        return false;
    }

    FileInStream_CreateVTable(&_archive_stream);
    _archive_stream.wres = 0;

    LookToRead2_CreateVTable(&_look_stream, False);
    _look_stream.buf = _buf;
    _look_stream.bufSize = LOOK_STREAM_BUF_SIZE;
    _look_stream.realStream = &_archive_stream.vt;
    LookToRead2_INIT(&_look_stream);

    SzArEx_Init(&_db);

    bool result = SzArEx_Open(&_db, &_look_stream.vt, &_alloc_imp, &_alloc_imp) == SZ_OK;
    if (!result)
    {
        Close();
    }

    return result && First();
}

void ArchiveReader7z::Close()
{
    SzArEx_Free(&_db, &_alloc_imp);
    File_Close(&_archive_stream.file);
}

bool ArchiveReader7z::First()
{
    _index = 0;
    return _db.NumFiles > 0;
}

bool ArchiveReader7z::Next()
{
    _index++;
    return _index < _db.NumFiles;
}

bool ArchiveReader7z::ExtractCurrent(const char *dst_path)
{
    if (SzArEx_IsDir(&_db, _index))
    {
        return false;
    }

    uint8_t *buf = nullptr;
    size_t buf_size = 0;
    uint32_t block_index = 0xffffffff;
    size_t offset = 0;
    size_t out_size = 0;
    bool result = (SzArEx_Extract(&_db,
                                  &_look_stream.vt,
                                  _index,
                                  &block_index,
                                  &buf,
                                  &buf_size,
                                  &offset,
                                  &out_size,
                                  &_alloc_imp,
                                  &_alloc_imp) == SZ_OK);
    if (result)
    {
        result = File::WriteFile(dst_path, buf, buf_size);
    }

    ISzAlloc_Free(&_alloc_imp, buf);
    return result;
}

const char *ArchiveReader7z::GetCurrentName()
{
    uint16_t tmp[SCE_FIOS_PATH_MAX];
    static char name[SCE_FIOS_PATH_MAX];
    SzArEx_GetFileNameUtf16(&_db, _index, tmp);
    Utils::Utf16ToUtf8(tmp, name, SCE_FIOS_PATH_MAX);
    return name;
}

uint32_t ArchiveReader7z::GetCurrentCrc32()
{
    return _db.CRCs.Vals[_index];
}

size_t ArchiveReader7z::GetCurrentSize()
{
    return SzArEx_GetFileSize(&_db, _index);
}