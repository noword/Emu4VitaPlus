#pragma once
#include <stdint.h>
#include <zlib.h>
#include "file.h"

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

    Crc32Cache()
    {
        _cache = new CRC32_CACHE[SIZE];
        memset(_cache, 0, _cache_bytes);
    }

    virtual ~Crc32Cache()
    {
        delete[] _cache;
    }

    bool Load(const char *path)
    {
        return File::ReadFile(path, _cache, _cache_bytes);
    }

    bool Save(const char *path)
    {
        return File::WriteFile(path, _cache, _cache_bytes);
    }

    uint32_t Get(const char *path)
    {
        if (!File::Exist(path))
            return 0;

        time_t time;
        File::GetModifyTime(path, &time);
        size_t size = File::GetSize(path);
        uint32_t key = crc32(0, (const Bytef *)path, strlen(path));
        key = crc32(key, (const Bytef *)&time, sizeof(time_t));
        key = crc32(key, (const Bytef *)&size, sizeof(size_t));
        uint32_t index = key & (SIZE - 1);
        if (_cache[index].key == key)
        {
            return _cache[index].crc32;
        }
        else
        {
            uint32_t crc32 = File::GetCrc32(path);
            _cache[index] = {key, crc32};
            return crc32;
        }
    }

private:
    CRC32_CACHE *_cache;
    const size_t _cache_bytes = SIZE * sizeof(CRC32_CACHE);
};
