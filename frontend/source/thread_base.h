#pragma once
#include <stdint.h>
#include <psp2/kernel/threadmgr.h>

#define CLASS_POINTER(CLASS, POINT, ARGP) CLASS *POINT = *(CLASS **)ARGP;

#define SCE_KERNEL_HIGHEST_PRIORITY_USER 64
#define SCE_KERNEL_LOWEST_PRIORITY_USER 191
#define SCE_KERNEL_DEFAULT_PRIORITY_USER 0x10000100

class ThreadBase
{
public:
    ThreadBase(SceKernelThreadEntry entry,
               int priority = SCE_KERNEL_DEFAULT_PRIORITY_USER,
               int cpu_affinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
               int stack_size = 0x10000);
    virtual ~ThreadBase();

    bool Start(); // the point of class will be sent
    bool Start(void *data, SceSize size);
    void Stop(bool force = false);
    bool IsRunning() { return _keep_running; };
    int32_t Lock(uint32_t *timeout = NULL);
    void Unlock();
    int32_t Wait(uint32_t *timeout = NULL);
    void Signal();

protected:
    SceKernelThreadEntry _entry;
    int _priority;
    int _cpu_affinity;
    SceSize _stack_size;
    bool _keep_running;
    SceUID _thread_id;
    SceKernelLwMutexWork _mutex;
    SceUID _semaid;
};

// entry function must call sceKernelExitDeleteThread at end
void StartThread(SceKernelThreadEntry entry, SceSize args, void *argp,
                 int priority = SCE_KERNEL_DEFAULT_PRIORITY_USER,
                 int cpu_affinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
                 int stack_size = 0x10000);