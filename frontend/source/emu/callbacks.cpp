#include <stdlib.h>
#include <stdint.h>
#include "emulator.h"
#include "video.h"
#include "ui.h"
#include "config.h"
#include "file.h"

#define CORE_OPTIONS_VERSION 2

void RetroLog(retro_log_level level, const char *fmt, ...)
{
    LogFunctionName;

    va_list list;
    char str[512];

    va_start(list, fmt);
    vsprintf(str, fmt, list);
    va_end(list);

    switch (level)
    {
    case RETRO_LOG_DEBUG:
        LogDebug(str);
        break;
    case RETRO_LOG_INFO:
        LogInfo(str);
        break;
    case RETRO_LOG_WARN:
        LogWarn(str);
        break;
    case RETRO_LOG_ERROR:
        LogWarn(str);
        break;
    }

    gUi->AppendLog(str);
}

bool EnvironmentCallback(unsigned cmd, void *data)
{
    LogFunctionNameLimited;
    LogTrace("  cmd: %u", cmd);

    switch (cmd)
    {
    case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_CAN_DUPE");
        if (data)
        {
            *(bool *)data = true;
        }
        break;
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY");
        if (data)
        {
            *(const char **)data = CORE_SYSTEM_DIR;
        }
        break;

    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_PIXEL_FORMAT");
        gEmulator->_SetPixelFormat(*(retro_pixel_format *)data);
        break;

    // case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    //     break;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS");
        gConfig->core_options.Load((retro_core_option_definition *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE");
        break;

    case RETRO_ENVIRONMENT_GET_VARIABLE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_VARIABLE");
        if (data)
        {
            return gConfig->core_options.Get((retro_variable *)data);
        }
        else
        {
            return false;
        }

        break;

    case RETRO_ENVIRONMENT_SET_VARIABLES:
        LogDebug("  RETRO_ENVIRONMENT_SET_VARIABLES: %s", ((retro_variable *)data)->key);
        break;

    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        // LogDebug("  cmd: RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
        *(bool *)data = false;
        break;

    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
        if (data)
        {
            ((retro_log_callback *)data)->log = RetroLog;
        }
        break;

    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
        if (data)
        {
            static char path[SCE_FIOS_PATH_MAX];
            snprintf(path, SCE_FIOS_PATH_MAX, CORE_SAVEFILES_DIR "/%s", gEmulator->_current_name.c_str());
            *(const char **)data = path;
            LogDebug("  return: %s", path);
        }
        break;

    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
        memcpy(&gEmulator->_av_info, data, sizeof(retro_system_av_info));
        gEmulator->_audio.SetSampleRate(gEmulator->_av_info.timing.sample_rate);
        break;

    case RETRO_ENVIRONMENT_GET_LANGUAGE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_LANGUAGE");
        if (data)
        {
            *(retro_language *)data = gConfig->GetRetroLanguage();
            LogDebug("  retro_language:%d", *(retro_language *)data);
        }
        break;

    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION");
        if (data)
            *(unsigned *)data = CORE_OPTIONS_VERSION;
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL");
        gConfig->core_options.Load((retro_core_options_intl *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL");
        gConfig->core_options.Load((retro_core_options_v2_intl *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
        return gEmulator->GetCurrentSoftwareFramebuffer((retro_framebuffer *)data);

    case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
        // LogDebug("  cmd: RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE");
        if (data)
        {
            *(int *)data = 3;
        }
        break;

    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_INPUT_BITMASKS");
        break;

    default:
        LogDebug("  unsupported cmd:%d", cmd);
        return false;
    }

    return true;
}

void VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    LogFunctionNameLimited;

    if (!data)
    {
        LogDebug("video data is NULL");
        return;
    }

    if (gEmulator->_graphics_config_changed || gEmulator->_texture_buf == nullptr || gEmulator->_texture_buf->GetWidth() != width || gEmulator->_texture_buf->GetHeight() != height)
    {
        gVideo->Lock();

        if (gEmulator->_texture_buf != nullptr)
        {

            delete gEmulator->_texture_buf;
            gEmulator->_texture_buf = nullptr;
        }
        gEmulator->_texture_buf = new TextureBuf(gEmulator->_video_pixel_format, width, height);
        gEmulator->_SetVideoSize(width, height);
        gEmulator->_graphics_config_changed = false;

        gVideo->Unlock();
    }
    else if (gEmulator->_soft_frame_buf_render)
    {
        sceKernelSignalSema(gEmulator->_video_semaid, 1);
        return;
    }

    gEmulator->_texture_buf->Lock();
    vita2d_texture *texture = gEmulator->_texture_buf->Next();

    unsigned out_pitch = vita2d_texture_get_stride(texture);
    uint8_t *out = (uint8_t *)vita2d_texture_get_datap(texture);
    uint8_t *in = (uint8_t *)data;

    if (pitch == out_pitch)
    {
        memcpy(out, in, pitch * height);
    }
    else
    {
        for (unsigned i = 0; i < height; i++)
        {
            memcpy(out, in, pitch);
            in += pitch;
            out += out_pitch;
        }
    }

    gEmulator->_texture_buf->Unlock();
    sceKernelSignalSema(gEmulator->_video_semaid, 1);
}

size_t AudioSampleBatchCallback(const int16_t *data, size_t frames)
{
    LogFunctionNameLimited;
    return gEmulator->_audio.SendAudioSample(data, frames);
}

void InputPollCallback()
{
    LogFunctionNameLimited;
}

int16_t InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    LogFunctionNameLimited;
    if (device != RETRO_DEVICE_JOYPAD || port != 0)
    {
        return 0;
    }
    // LogDebug("%d %d %d %d", port, device, index, id);
    uint32_t key_states = gEmulator->_input.GetKeyStates();

    if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
    {
        gEmulator->_input.ClearKeyStates(gEmulator->_keys_mask);
        uint64_t keys = key_states;
        int16_t state = 0;
        for (size_t i = 0; i < 16; i++)
        {
            if (keys & gEmulator->_keys[i])
            {
                state |= (1 << i);
            }
        }
        return state;
    }
    else if (id >= 16)
    {
        LogError("  InputStateCallback, wrong id %d", id);
        return 0;
    }
    else
    {
        // gEmulator->_input.ClearKeyStates(gEmulator->_keys[id]);
        return key_states & gEmulator->_keys[id];
    }
}
