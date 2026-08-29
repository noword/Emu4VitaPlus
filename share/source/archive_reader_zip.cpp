#include "archive_reader_zip.h"
#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>

ArchiveReaderZip::ArchiveReaderZip()
{
    _handle = mz_zip_reader_create();
}

ArchiveReaderZip::~ArchiveReaderZip()
{
    Close();
    mz_zip_reader_delete(&_handle);
}

bool ArchiveReaderZip::Open(const char *path)
{
    return mz_zip_reader_open_file(_handle, path) == MZ_OK && First();
}

void ArchiveReaderZip::Close()
{
    mz_zip_reader_close(_handle);
}

bool ArchiveReaderZip::First()
{
    return mz_zip_reader_goto_first_entry(_handle) == MZ_OK;
}

bool ArchiveReaderZip::Next()
{
    return mz_zip_reader_goto_next_entry(_handle) == MZ_OK;
}

bool ArchiveReaderZip::ExtractCurrent(const char *dst_path)
{
    return mz_zip_reader_entry_save_file(_handle, dst_path) == MZ_OK;
}

const char *ArchiveReaderZip::GetCurrentName()
{
    mz_zip_file *info;
    mz_zip_reader_entry_get_info(_handle, &info);
    return info->filename;
}

uint32_t ArchiveReaderZip::GetCurrentCrc32()
{
    mz_zip_file *info;
    mz_zip_reader_entry_get_info(_handle, &info);
    return info->crc;
}

size_t ArchiveReaderZip::GetCurrentSize()
{
    mz_zip_file *info;
    mz_zip_reader_entry_get_info(_handle, &info);
    return info->uncompressed_size;
}
