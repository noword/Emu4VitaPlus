#pragma once
#include <stdint.h>
#include <string>
#include "locker.h"
#include "singleton.h"

#define CLASS_POINTER(CLASS, POINT, ARGP) CLASS *POINT = *(CLASS **)ARGP;

#define SCE_KERNEL_HIGHEST_PRIORITY_USER 64
#define SCE_KERNEL_LOWEST_PRIORITY_USER 191
#define SCE_KERNEL_DEFAULT_PRIORITY_USER 0x10000100
#define DEFAULT_STACK_SIZE 0x100000

class ThreadBase : public Locker, public Singleton
{
public:
    ThreadBase(SceKernelThreadEntry entry,
               const char *name = nullptr,
               int priority = SCE_KERNEL_DEFAULT_PRIORITY_USER,
               int cpu_affinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
               int stack_size = DEFAULT_STACK_SIZE);
    virtual ~ThreadBase();

    bool Start(); // the point of class will be sent
    bool Start(void *data, SceSize size);
    void Stop(bool force = false);
    bool IsRunning() { return _keep_running; };

protected:
    SceKernelThreadEntry _entry;
    const std::string _name;
    int _priority;
    int _cpu_affinity;
    SceSize _stack_size;
    bool _keep_running;
    SceUID _thread_id;
};

// entry function must call sceKernelExitDeleteThread at end
void StartThread(SceKernelThreadEntry entry, SceSize args, void *argp,
                 int priority = SCE_KERNEL_DEFAULT_PRIORITY_USER,
                 int cpu_affinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
                 int stack_size = DEFAULT_STACK_SIZE);