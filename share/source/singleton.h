#pragma once
#include <psp2/kernel/threadmgr.h>

class Singleton
{
public:
    Singleton(const char *name = "sema")
    {
        _semaid = sceKernelCreateSema(name, 0, 0, 1, NULL);
    }

    virtual ~Singleton()
    {
        sceKernelDeleteSema(_semaid);
    }

    int32_t Wait(uint32_t *timeout = NULL)
    {
        return sceKernelWaitSema(_semaid, 1, timeout);
    }

    void Signal()
    {
        sceKernelSignalSema(_semaid, 1);
    }

private:
    SceUID _semaid;
};