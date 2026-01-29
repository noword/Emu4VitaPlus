#include <string.h>
#include "hw_render.h"
#include "log.h"

#if !defined(ENABLE_VITAGL)
HardwareRender::HardwareRender(retro_hw_render_callback *data) {}
HardwareRender::~HardwareRender() {}
retro_hw_render_callback *HardwareRender::GetRetroHwRenderCallback() {}

#else
bool HardwareRender ::_inited = false;

static GLuint static_hw_frame_buffer = 0;
uintptr_t GetCurrentFrameBuffer()
{
    LogFunctionNameLimited;
    return static_hw_frame_buffer;
}

HardwareRender::HardwareRender(retro_hw_render_callback *data)
{
    LogFunctionName;

    if (!_inited)
    {
        _inited = true;
        vglInitExtended(0, 960, 544, 4 * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);
    }

    data->get_current_framebuffer = GetCurrentFrameBuffer;
    data->get_proc_address = (retro_hw_get_proc_address_t)vglGetProcAddress;
    data->version_major = 2;
    data->version_minor = 0;

    memcpy(&_retro_hw_render_callback, data, sizeof(retro_hw_render_callback));
}

HardwareRender::~HardwareRender()
{
    LogFunctionName;
}

retro_hw_render_callback *HardwareRender::GetRetroHwRenderCallback()
{
    return &_retro_hw_render_callback;
}
#endif