#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "app.h"
#include "emulator.h"
#include "video.h"
#include "ui.h"
#include "config.h"
#include "file.h"
#include "profiler.h"
#include "input_descriptor.h"
#include "performance.h"

#define CORE_OPTIONS_VERSION 2

void RetroLog(retro_log_level level, const char *fmt, ...)
{
    // #if LOG_LEVEL > LOG_LEVEL_DEBUG
    if (gStatus.Get() == APP_STATUS_RUN_GAME)
    {
        return;
    }
    // #endif

    va_list list;
    char str[512];

    va_start(list, fmt);
    vsnprintf(str, 512, fmt, list);
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

    if (level >= RETRO_LOG_INFO && gStatus.Get() != APP_STATUS_RUN_GAME)
    {
        gUi->AppendLog(str);
    }
}

bool EnvironmentCallback(unsigned cmd, void *data)
{
    LogFunctionNameLimited;
    LogTrace("  cmd: %u", cmd);

    switch (cmd)
    {
    case RETRO_ENVIRONMENT_SET_ROTATION:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_ROTATION");
        {
            VIDEO_ROTATION *rotation = (VIDEO_ROTATION *)data;
            LogDebug("  *data: %d", *rotation);
            if (!gConfig->auto_rotating)
            {
                LogDebug("  disable auto rotating");
                return false;
            }

            if (gEmulator->_video_rotation != *rotation)
            {

                gEmulator->_video_rotation = *rotation;
                gEmulator->ChangeGraphicsConfig();
                gEmulator->SetupKeys();
            }
        }
        // TODO: Support Rotation
        break;

    case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_CAN_DUPE");
        if (data)
        {
            *(bool *)data = true;
        }
        break;

    case RETRO_ENVIRONMENT_SET_MESSAGE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_MESSAGE");
        {
            retro_message *message = (retro_message *)data;
            LogDebug(message->msg);
            gUi->SetHint(message->msg);
        }
        break;

    case RETRO_ENVIRONMENT_SHUTDOWN:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SHUTDOWN");
        gEmulator->UnloadGame();
        break;

    case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL");
        LogDebug("  PERFORMANCE_LEVEL:%f", *(const unsigned *)data);
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

    case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS");
        gConfig->input_descriptors.UpdateInputDescriptors((retro_input_descriptor *)data);
        gConfig->Save();
        break;

    case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK");
        gEmulator->_keyboard->SetCallback(data ? ((const retro_keyboard_callback *)data)->callback : nullptr);
        break;

    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE");
        gEmulator->SetDiskControlCallback((const retro_disk_control_callback *)data);
        gUi->UpdateDiskOptions();
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
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_VARIABLES");
        gConfig->core_options.Load((retro_variable *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        // LogDebug("  cmd: RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
        *(bool *)data = gEmulator->_core_options_updated;
        gEmulator->_core_options_updated = false;
        break;

    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME");
        if (data)
        {
            gConfig->support_no_game = *(bool *)data;
        }
        break;

    case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK");
        return false;

    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
        if (data)
        {
            ((retro_log_callback *)data)->log = RetroLog;
        }
        break;

    case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
        {
            retro_perf_callback *cb = (struct retro_perf_callback *)data;
            cb->get_time_usec = GetTimeUsec;
            cb->get_cpu_features = GetCpuFeatures;
            cb->get_perf_counter = GetPerfCounter;
            cb->perf_register = RegisterPerfCounter;
            cb->perf_start = PerfCounterStart;
            cb->perf_stop = PerfCounterStop;
            cb->perf_log = PerfLog;
        }

        break;

    case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY");
        if (data)
            *(const char **)data = CORE_SAVEFILES_DIR;
        break;

    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
        if (data)
        {
            static char path[SCE_FIOS_PATH_MAX];
            snprintf(path, SCE_FIOS_PATH_MAX, "%s/%s", CORE_SAVEFILES_DIR, gEmulator->_current_name.c_str());
            if (!File::Exist(path))
            {
                File::MakeDirs(path);
            }
            *(const char **)data = path;
            LogDebug("  return: %s", path);
        }
        break;

    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
        memcpy(&gEmulator->_av_info, data, sizeof(retro_system_av_info));
        // gEmulator->_audio.Init(gEmulator->_av_info.timing.sample_rate);
        gEmulator->SetSpeed(gEmulator->_speed);
        break;

    case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO");
        return false;

    case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CONTROLLER_INFO");
        gEmulator->_SetControllerInfo((retro_controller_info *)data);
        break;

    case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_SET_MEMORY_MAPS");
        return false;

    case RETRO_ENVIRONMENT_SET_GEOMETRY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_GEOMETRY");
        gEmulator->ChangeGraphicsConfig();
        break;

    case RETRO_ENVIRONMENT_GET_LANGUAGE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_LANGUAGE");
        if (data)
        {
            *(retro_language *)data = gConfig->GetRetroLanguage();
            LogDebug("  retro_language:%d", *(retro_language *)data);
        }
        break;

    case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS");
        return false;

    case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS");
        return false;

    case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_GET_VFS_INTERFACE");
        return false;

    case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_LED_INTERFACE");
        if (data)
        {
            ((retro_led_interface *)data)->set_led_state = NULL;
        }
        break;

    case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
        *(bool *)data = gEmulator->_speed > 1.0;
        break;

    case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
        LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE");
        return false;

    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION");
        if (data)
            *(unsigned *)data = CORE_OPTIONS_VERSION;
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS");
        gConfig->core_options.Load((retro_core_option_definition *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL");
        gConfig->core_options.Load((retro_core_options_intl *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY");
        gConfig->core_options.SetVisable((const retro_core_option_display *)data);
        break;

    case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION");
        if (data)
            *(unsigned *)data = 1;
        break;

    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE");
        gEmulator->SetDiskControlCallback((const retro_disk_control_ext_callback *)data);
        gUi->UpdateDiskOptions();
        break;

    case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION");
        *(unsigned *)data = 1;
        break;

    case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_MESSAGE_EXT");
        {
            retro_message_ext *message = (retro_message_ext *)data;
            LogDebug(message->msg);
            gUi->SetHint(message->msg);
        }
        break;

    case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK");
        gEmulator->_audio.SetBufStatusCallback(data ? ((const retro_audio_buffer_status_callback *)data)->callback : nullptr);
        break;

    case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY");
        if (data && *(const unsigned *)data != 0)
        {
            LogDebug("  data: %d", *(const unsigned *)data);
            gEmulator->SetSpeed(gEmulator->_speed); // will reset audio
        }
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2");
        gConfig->core_options.Load((retro_core_options_v2 *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL");
        gConfig->core_options.Load((retro_core_options_v2_intl *)data);
        gUi->UpdateCoreOptions();
        break;

    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
        LogDebug("  cmd: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK");
        gEmulator->_core_options_update_display_callback = data ? ((const retro_core_options_update_display_callback *)data)->callback : nullptr;
        break;

    case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
        return gEmulator->GetCurrentSoftwareFramebuffer((retro_framebuffer *)data);

    case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
        // LogDebug("  cmd: RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE");
        if (data)
        {
            *(int *)data = gEmulator->_show_video ? RETRO_AV_ENABLE_VIDEO : 0;
            if ((!gConfig->mute) && gStatus.Get() == APP_STATUS_RUN_GAME)
                *(int *)data |= RETRO_AV_ENABLE_AUDIO;
        }
        break;

    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
        LogDebug("  cmd: RETRO_ENVIRONMENT_GET_INPUT_BITMASKS");
        break;

    case RETRO_ENVIRONMENT_GET_THROTTLE_STATE:
    {
        retro_throttle_state *throttle_state = (struct retro_throttle_state *)data;
        switch (gStatus.Get())
        {
        case APP_STATUS_SHOW_UI_IN_GAME:
            throttle_state->mode = RETRO_THROTTLE_FRAME_STEPPING;
            throttle_state->rate = 0.0f;
            break;

        case APP_STATUS_REWIND_GAME:
            throttle_state->mode = RETRO_THROTTLE_REWINDING;
            throttle_state->rate = 0.0f;
            break;

        case APP_STATUS_RUN_GAME:
        default:
            throttle_state->mode = RETRO_THROTTLE_NONE;
            throttle_state->rate = std::min((float)gEmulator->_av_info.timing.fps, 60.0);
            break;
        }
    }

    case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT:
        // LogDebug("  cmd: RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT");
        if (data)
        {
            *(retro_savestate_context *)data = RETRO_SAVESTATE_CONTEXT_NORMAL;
        }
        break;

    default:
        if (cmd > RETRO_ENVIRONMENT_EXPERIMENTAL)
        {
            LogDebug("  unsupported cmd: RETRO_ENVIRONMENT_EXPERIMENTAL | %d", cmd - RETRO_ENVIRONMENT_EXPERIMENTAL);
        }
        else
        {
            LogDebug("  unsupported cmd: %d", cmd);
        }
        return false;
    }

    return true;
}
