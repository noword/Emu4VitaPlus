#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <zlib.h>
#include <string.h>
#include "arcade_manager.h"
#include "file.h"
#include "log.h"
#include "emulator.h"
#include "archive_reader_factory.h"
#include "file.h"

#if defined(FBA_BUILD)
#define DAT_BIN_PATH "app0:assets/fba_dat.zbin"
#elif defined(MAME_BUILD)
#define DAT_BIN_PATH "app0:assets/mame_dat.zbin"
#endif

#define ARC_CACHE_MAX_SIZE 10

void Emulator::_InitArcadeManager()
{
#ifdef ARC_BUILD
    _arcade_manager = new ArcadeManager();
#endif
}

#ifdef ARC_BUILD
ArcadeManager::ArcadeManager() : CacheManager(ARCADE_CACHE_DIR, ARC_CACHE_MAX_SIZE), _names(nullptr)
{
    _Load();
}

bool ArcadeManager::NeedReplace(const char *path)
{
    char tmp[SCE_FIOS_PATH_MAX];
    strncpy(tmp, path, SCE_FIOS_PATH_MAX);
    char *p = strrchr(tmp, '.');
    if (p)
    {
        strcpy(p, ".dat");
        return !File::Exist(tmp);
    }
    else
    {
        return true;
    }
}

ArcadeManager::~ArcadeManager()
{
    if (_names)
    {
        delete[] _names;
    }
}

void ArcadeManager::_Load()
{
    LogFunctionName;
    uint32_t *buf;
    if (File::ReadCompressedFile(DAT_BIN_PATH, (void **)&buf) == 0)
    {
        LogError("failed to load %s", DAT_BIN_PATH);
        return;
    }
    uint32_t *p = buf;

    uint32_t size = *p++;
    _names = new char[size];
    memcpy(_names, p, size);
    p += size / sizeof(uint32_t);

    size = *p++;
    _name_hash.insert(p, p + size);
    p += size;

    size = *p++;

    for (size_t i = 0; i < size; i++)
    {
        uint32_t n = *p++;
        uint32_t crc32 = *p++;
        _roms[crc32] = std::vector<uint32_t>{p, p + n};
        p += n;
    }

    delete[] buf;
}

const char *ArcadeManager::GetRomName(const char *path) const
{
    LogFunctionName;

    std::string stem = File::GetStem(path);
    if (_name_hash.find(crc32(0, (Bytef *)stem.c_str(), stem.size())) != _name_hash.end())
    {
        return path;
    }

    ArchiveReader *reader = gArchiveReaderFactory->Get(path);
    if (reader == nullptr || !reader->Open(path))
    {
        return path;
    }

    std::unordered_map<uint32_t, uint32_t> offsets; // offset:count
    do
    {
        uint32_t crc32 = reader->GetCurrentCrc32();
        auto iter = _roms.find(crc32);
        if (iter == _roms.end())
        {
            continue;
        }

        for (const auto &offset : iter->second)
        {
            if (offsets.find(offset) == offsets.end())
            {
                offsets[offset] = 1;
            }
            else
            {
                offsets[offset] += 1;
            }
        }
    } while (reader->Next());

    if (offsets.size() == 0)
    {
        return path;
    }

    uint32_t offset = 0;
    uint32_t max_count = 0;
    for (const auto &iter : offsets)
    {
        LogDebug("%08x %s %d", iter.first, _names + iter.first, iter.second);
        if (iter.second > max_count)
        {
            offset = iter.first;
            max_count = iter.second;
        }
    }

    return _names + offset;
}

const char *ArcadeManager::GetCachedPath(const char *path)
{
    LogFunctionName;

    const char *name = GetRomName(path);
    if (name == path)
    {
        return path;
    }

    char cached_path[SCE_FIOS_PATH_MAX];
    static char cached_full_path[SCE_FIOS_PATH_MAX];
    snprintf(cached_path, SCE_FIOS_PATH_MAX, "%s.%s", name, File::GetExt(path).c_str());
    snprintf(cached_full_path, SCE_FIOS_PATH_MAX, "%s/%s ", ARCADE_CACHE_DIR, cached_path);
    LogDebug("%s %s", cached_path, cached_full_path);
    if (this->IsInCache(cached_path))
    {
        LogDebug("  %s in cache", cached_path);
        return cached_full_path;
    }
    else if (File::CopyFile(path, cached_full_path))
    {
        LogDebug("  copy %s to %s", path, cached_full_path);
        Set(cached_path);
        return cached_full_path;
    }
    else
    {
        return path;
    }
}
#else
ArcadeManager::ArcadeManager() : CacheManager(ARCADE_CACHE_DIR, ARC_CACHE_MAX_SIZE) {}
ArcadeManager::~ArcadeManager() {}
const char *ArcadeManager::GetRomName(const char *path) const { return NULL; }
const char *ArcadeManager::GetCachedPath(const char *path) { return NULL; }
bool ArcadeManager::NeedReplace(const char *path) { return false; }
void ArcadeManager_Load() {}
#endif