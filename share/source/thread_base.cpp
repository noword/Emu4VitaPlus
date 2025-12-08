#include <stdint.h>
#include "thread_base.h"
#include "log.h"

ThreadBase::ThreadBase(SceKernelThreadEntry entry, const char *name, int priority, int cpu_affinity, int stack_size)
    : Locker{"thread_mutex"},
      Singleton{"thread_sema"},
      _entry(entry),
      _name(name ? name : __PRETTY_FUNCTION__),
      _priority(priority),
      _cpu_affinity(cpu_affinity),
      _stack_size(stack_size),
      _thread_id(-1),
      _keep_running(false)
{
    LogFunctionName;
}

ThreadBase::~ThreadBase()
{
    LogFunctionName;
    if (_thread_id >= 0)
    {
        Stop(true);
    }
}

bool ThreadBase::Start()
{
    LogFunctionName;

    uint32_t p = (uint32_t)this;
    return Start(&p, 4);
}

bool ThreadBase::Start(void *data, SceSize size)
{
    LogFunctionName;

    _thread_id = sceKernelCreateThread(_name.c_str(), _entry, _priority, _stack_size, 0, _cpu_affinity, NULL);
    if (_thread_id < 0)
    {
        LogError("failed to create thread: %s", _name.c_str());
        return false;
    }

    _keep_running = true;
    int result = sceKernelStartThread(_thread_id, size, data);
    if (result != SCE_OK)
    {
        LogError("failed to start thread: %s / %d", _name.c_str(), result);
        sceKernelDeleteThread(_thread_id);
        _thread_id = -1;
        _keep_running = false;
        return false;
    }

    LogInfo("Thread '%s' started. id: %08x function: %08x", _name.c_str(), _thread_id, _entry);

    return true;
}

void ThreadBase::Stop(bool force)
{
    LogFunctionName;
    LogDebug("%08x %d", _thread_id, force);
    if (_thread_id == -1)
    {
        return;
    }

    _keep_running = false;
    Signal();
    if (force)
    {
        sceKernelDelayThread(10000);
    }
    else
    {
        sceKernelWaitThreadEnd(_thread_id, NULL, NULL);
    }

    int result = sceKernelDeleteThread(_thread_id);
    if (result != SCE_OK)
    {
        LogError("sceKernelDeleteThread error: %08x %08x", _thread_id, result);
    }

    LogDebug("thread %08x exited", _thread_id);
    _thread_id = -1;
}

void StartThread(SceKernelThreadEntry entry,
                 SceSize args,
                 void *argp,
                 int priority,
                 int cpu_affinity,
                 int stack_size)
{
    SceUID thread_id = sceKernelCreateThread(__PRETTY_FUNCTION__, entry, priority, stack_size, 0, cpu_affinity, NULL);
    sceKernelStartThread(thread_id, args, argp);
}