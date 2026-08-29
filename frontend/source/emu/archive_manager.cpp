#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>
#include <7z.h>
#include <7zFile.h>
#include <7zAlloc.h>
#include <7zCrc.h>
#include "archive_manager.h"
#include "file.h"
#include "utils.h"
#include "defines.h"
#include "log.h"
#include "archive_reader_factory.h"

#define DEFAULT_CACHE_SIZE 10

ArchiveManager::ArchiveManager() : CacheManager(ARCHIVE_CACHE_DIR, DEFAULT_CACHE_SIZE)
{
}

ArchiveManager::~ArchiveManager()
{
}

const char *ArchiveManager::GetCachedPath(uint32_t crc32, const char *name, const char *entry_name)
{
    LogFunctionName;
    static std::string full_path;
    if (IsInCache(_GetCachedName(crc32, entry_name).c_str()))
    {
        full_path = _GetCachedFullName(crc32, entry_name);
        LogDebug("  return: %s", full_path.c_str());
        return full_path.c_str();
    }

    if (!name || !entry_name)
    {
        return nullptr;
    }

    ArchiveReader *reader = gArchiveReaderFactory->Get(name);
    if (reader == nullptr || !reader->Open(name))
    {
        return nullptr;
    }

    full_path = _GetCachedFullName(crc32, entry_name);
    if (!reader->ExtractByHash(crc32, full_path.c_str()))
    {
        return nullptr;
    }

    Set(_GetCachedName(crc32, entry_name).c_str());
    LogDebug("  return: %s", full_path.c_str());
    return full_path.c_str();
}

std::string ArchiveManager::_GetCachedName(uint32_t crc32, const char *entry_name)
{
    char path[SCE_FIOS_PATH_MAX];
    snprintf(path, SCE_FIOS_PATH_MAX, "%08x%s", crc32, strrchr(entry_name, '.'));
    return std::string(path);
}

std::string ArchiveManager::_GetCachedFullName(uint32_t crc32, const char *entry_name)
{
    return GetDirPath() + "/" + _GetCachedName(crc32, entry_name);
}