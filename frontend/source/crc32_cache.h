#pragma once
#include <stdint.h>
#include <zlib.h>
#include "file.h"
#include "defines.h"
#include "log.h"

#define DEFAULT_CRC32_CACHE_SIZE 0x2000

#define IS_POWER_OF_TWO(x) ((x) > 0 && (((x) & ((x) - 1)) == 0))

struct CRC32_CACHE
{
    uint32_t key; // crc32(fullpath + modify time + size)
    uint32_t crc32;
};

template <size_t SIZE = DEFAULT_CRC32_CACHE_SIZE>
class Crc32Cache
{
public:
    _Static_assert(IS_POWER_OF_TWO(SIZE), "cache size must be power of two");

    Crc32Cache() : _save_count(0)
    {
        _cache = new CRC32_CACHE[SIZE];
        memset(_cache, 0, _cache_bytes);
    }

    virtual ~Crc32Cache()
    {
        delete[] _cache;
    }

    bool Load(const char *path = CRC32_CACHE_FILE)
    {
        LogFunctionName;
        return File::ReadFile(path, _cache, _cache_bytes);
    }

    bool Save(const char *path = CRC32_CACHE_FILE)
    {
        LogFunctionName;
        return File::WriteFile(path, _cache, _cache_bytes);
    }

    uint32_t Get(const char *path)
    {
        LogFunctionName;
        if (!File::Exist(path))
            return 0;

        time_t time;
        File::GetModifyTime(path, &time);
        size_t size = File::GetSize(path);

        uint32_t key = crc32(0, (const Bytef *)path, strlen(path));
        uint32_t index = key & (SIZE - 1);
        key = crc32(key, (const Bytef *)&time, sizeof(time_t));
        key = crc32(key, (const Bytef *)&size, sizeof(size_t));
        if (_cache[index].key == key)
        {
            LogDebug("hit cache %x: %s[0x%08x]", index, path, _cache[index].crc32);
            return _cache[index].crc32;
        }
        else
        {
            uint32_t crc32 = File::GetCrc32(path);
            _cache[index] = {key, crc32};

            _save_count++;
            if (_save_count > 10)
            {
                _save_count = 0;
                Save();
            }

            LogDebug("miss cache %d: %s[0x%08x]", index, path, crc32);

            return crc32;
        }
    }

private:
    CRC32_CACHE *_cache;
    const size_t _cache_bytes = SIZE * sizeof(CRC32_CACHE);
    int _save_count;
};
