#include <stdint.h>
#include "thread_base.h"

ThreadBase::ThreadBase(SceKernelThreadEntry entry, int priority, int cpu_affinity, int stack_size)
    : _entry(entry),
      _priority(priority),
      _cpu_affinity(cpu_affinity),
      _stack_size(stack_size),
      _thread_id(-1),
      _keep_running(false)
{
    LogFunctionName;
    sceKernelCreateLwMutex(&_mutex, "thread_mutex", 0, 0, NULL);
    _semaid = sceKernelCreateSema("thread_sema", 0, 0, 1, NULL);
}

ThreadBase::~ThreadBase()
{
    LogFunctionName;
    if (_thread_id >= 0)
    {
        Stop(true);
    }
    sceKernelDeleteLwMutex(&_mutex);
    sceKernelDeleteSema(_semaid);
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

    _thread_id = sceKernelCreateThread(__PRETTY_FUNCTION__, _entry, _priority, _stack_size, 0, _cpu_affinity, NULL);
    if (_thread_id < 0)
    {
        LogError("failed to create thread: %s", __PRETTY_FUNCTION__);
        return false;
    }

    _keep_running = true;
    int result = sceKernelStartThread(_thread_id, size, data);
    if (result != SCE_OK)
    {
        LogError("failed to start thread: %s / %d", __PRETTY_FUNCTION__, result);
        sceKernelDeleteThread(_thread_id);
        _thread_id = -1;
        _keep_running = false;
        return false;
    }

    LogInfo("Thread started. id: %08x function: %08x", _thread_id, _entry);

    return true;
}

void ThreadBase::Stop(bool force)
{
    LogFunctionName;
    LogDebug("%08x", _thread_id);
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

int32_t ThreadBase::Lock(uint32_t *timeout)
{
    return sceKernelLockLwMutex(&_mutex, 1, timeout);
}

void ThreadBase::Unlock()
{
    sceKernelUnlockLwMutex(&_mutex, 1);
}

int32_t ThreadBase::Wait(uint32_t *timeout)
{
    return sceKernelWaitSema(_semaid, 1, timeout);
}

void ThreadBase::Signal()
{
    sceKernelSignalSema(_semaid, 1);
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