#include <psp2/kernel/threadmgr.h>
#include "profiler.h"
#include "log.h"

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
Profiler *gProfiler;

Profiler::Profiler(uint64_t log_interval) : _log_interval(log_interval)
{
}

Profiler::~Profiler()
{
}

void Profiler::BeginBlock(const std::string &name)
{
    uint64_t now = sceKernelGetProcessTimeWide();
    auto iter = _blocks.find(name);
    if (iter == _blocks.end())
    {
        _blocks[name] = {now, 0, now + _log_interval};
    }
    else
    {
        iter->second.start_time = now;
    }
}

void Profiler::EndBlock(const std::string &name)
{
    auto iter = _blocks.find(name);
    if (iter == _blocks.end())
    {
        return;
    }

    uint64_t now = sceKernelGetProcessTimeWide();
    Block *block = &iter->second;
    block->working_time += now - block->start_time;

    if (now >= block->next_log_time)
    {
        SceKernelThreadInfo info{0};
        info.size = sizeof(SceKernelThreadInfo);
        sceKernelGetThreadInfo(sceKernelGetThreadId(), &info);
        LogDebug("Profiler: [%08x, %d] %s %0.4f %2.2f%%",
                 sceKernelGetThreadId(),
                 info.currentCpuId,
                 name.c_str(),
                 block->working_time / 1000000.0,
                 block->working_time * 100.0 / _log_interval);

        block->working_time = 0;
        block->next_log_time = now + _log_interval;
    }
}

extern "C"
{
    void StartProfile(const char *name)
    {
        gProfiler->BeginBlock(name);
    }

    void StopProfile(const char *name)
    {
        gProfiler->EndBlock(name);
    }
}
#endif