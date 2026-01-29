#pragma once
#include <libretro.h>
#include "defines.h"

#if defined(ENABLE_VITAGL)
#include <vitaGL.h>
#endif

class HardwareRender
{
public:
    HardwareRender(retro_hw_render_callback *data);
    virtual ~HardwareRender();

    retro_hw_render_callback *GetRetroHwRenderCallback();

private:
#if defined(ENABLE_VITAGL)
    static bool _inited;
    retro_hw_render_callback _retro_hw_render_callback;
#endif
};