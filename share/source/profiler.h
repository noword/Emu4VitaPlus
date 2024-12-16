#pragma once
#include <unordered_map>
#include <string>
#include <stdint.h>

#if LOG_LEVEL <= LOG_LEVEL_DEBUG

#define DEFAULT_LOG_INTERVAL 5000000

struct Block
{
    uint64_t start_time;
    uint64_t working_time;
    uint64_t next_log_time;
};

class Profiler
{
public:
    Profiler(uint64_t log_interval = DEFAULT_LOG_INTERVAL);
    virtual ~Profiler();

    void BeginBlock(const std::string &name);
    void EndBlock(const std::string &name);

private:
    std::unordered_map<std::string, Block> _blocks;
    uint64_t _log_interval;
};

extern Profiler *gProfiler;

#define BeginProfile(NAME) gProfiler->BeginBlock(NAME);
#define EndProfile(NAME) gProfiler->EndBlock(NAME);

#ifdef __cplusplus
extern "C"
{
#endif

    void StartProfile(const char *name);
    void StopProfile(const char *name);

#ifdef __cplusplus
}
#endif

#else

#define BeginProfile(NAME)
#define EndProfile(NAME)

#endif