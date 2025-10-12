#pragma once
#include <psp2/kernel/threadmgr.h>

class Locker
{
public:
    Locker(const char *name = "mutex")
    {
        sceKernelCreateLwMutex(&_mutex, name, 0, 0, NULL);
    }

    virtual ~Locker()
    {
        sceKernelDeleteLwMutex(&_mutex);
    }

    int32_t Lock(uint32_t *timeout = NULL)
    {
        return sceKernelLockLwMutex(&_mutex, 1, timeout);
    }

    void Unlock()
    {
        sceKernelUnlockLwMutex(&_mutex, 1);
    }

private:
    SceKernelLwMutexWork _mutex;
};