#include <psp2/kernel/processmgr.h>
#include "fps.h"

#define FPS_SAMPLES 32
#define ONE_SECOND 1000000

Fps::Fps() : _fps(0)
{
    _last_time = sceKernelGetProcessTimeWide();
}

Fps::~Fps()
{
}

void Fps::Update()
{
    _frame_count++;
    if (_frame_count >= FPS_SAMPLES)
    {
        uint64_t now = sceKernelGetProcessTimeWide();
        uint64_t delta = now - _last_time;
        _fps = ONE_SECOND / (delta / FPS_SAMPLES);
        _last_time = now;
        _frame_count = 0;
    }
}