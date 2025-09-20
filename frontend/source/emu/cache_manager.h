#pragma once
#include <string>
#include <unordered_map>
#include <time.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include "file.h"
#include "log.h"

class CacheManager
{
public:
    CacheManager(const char *dir_path, size_t max_size) : _dir_path(dir_path), _max_size(max_size)
    {
        if (File::Exist(dir_path))
        {
            SceUID dfd = sceIoDopen(dir_path);
            SceIoDirent dir;
            while (sceIoDread(dfd, &dir) > 0)
            {
                if (SCE_S_ISREG(dir.d_stat.st_mode))
                {
                    time_t time;
                    std::string name = _dir_path + "/" + dir.d_name;
                    File::GetModifyTime(name.c_str(), &time);
                    _cache[dir.d_name] = time;
                    LogDebug("  cached: %s %u", name.c_str(), time);
                }
            }
            sceIoDclose(dfd);
            CheckSize();
        }
        else
        {
            File::MakeDirs(dir_path);
        }
    };

    virtual ~CacheManager() {};

    bool IsInCache(const char *name) const { return _cache.find(name) != _cache.end(); };

    const std::string GetDirPath() const { return _dir_path; };

    void Set(const char *name, SceDateTime sdtime = {0})
    {
        if (sdtime.year == 0)
        {
            sceRtcGetCurrentClockLocalTime(&sdtime);
        }

        time_t time;
        sceRtcGetTime_t(&sdtime, &time);
        _cache[name] = time;
        CheckSize();
    }

    void CheckSize()
    {
        if (_cache.size() <= _max_size)
        {
            return;
        }

        const std::string *earliest_name = &_cache.begin()->first;
        const time_t *earliest_time = &_cache.begin()->second;
        for (const auto &iter : _cache)
        {
            if (iter.second < *earliest_time)
            {
                earliest_name = &iter.first;
                earliest_time = &iter.second;
            }
        }

        sceIoRemove(earliest_name->c_str());
        _cache.erase(*earliest_name);
    };

private:
    size_t _max_size;
    std::string _dir_path;
    std::unordered_map<std::string, time_t> _cache;
};