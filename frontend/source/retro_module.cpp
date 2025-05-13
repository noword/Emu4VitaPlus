#include <psp2/kernel/modulemgr.h>
#include <libretro.h>
#include "retro_module.h"
#include "file.h"
#include "log.h"

#define LOG_DEBUG_FUNC_ADDR(FUNC) LogDebug("%08x " #FUNC, FUNC);

RetroModule::RetroModule(const char *name)
{
    LogFunctionName;

    if (sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL) < 0)
    {
        LogError("failed to load libfios2.suprx");
        return;
    }

    if (sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL) < 0)
    {
        LogError("failed to load libc.suprx");
        return;
    }

    char module[SCE_FIOS_PATH_MAX];
    snprintf(module, SCE_FIOS_PATH_MAX, "app0:modules/%s.suprx", name);
    _id = sceKernelLoadStartModule(module, 0, NULL, 0, NULL, NULL);
    if (_id < 0)
    {
        LogError("Failed to load module %s: %d", module, _id);
        return;
    }

    LOG_DEBUG_FUNC_ADDR(retro_init);
    LOG_DEBUG_FUNC_ADDR(retro_deinit);
    LOG_DEBUG_FUNC_ADDR(retro_api_version);
    LOG_DEBUG_FUNC_ADDR(retro_get_system_info);
    LOG_DEBUG_FUNC_ADDR(retro_get_system_av_info);
    LOG_DEBUG_FUNC_ADDR(retro_set_controller_port_device);
    LOG_DEBUG_FUNC_ADDR(retro_reset);
    LOG_DEBUG_FUNC_ADDR(retro_run);
    LOG_DEBUG_FUNC_ADDR(retro_serialize_size);
    LOG_DEBUG_FUNC_ADDR(retro_serialize);
    LOG_DEBUG_FUNC_ADDR(retro_unserialize);
    LOG_DEBUG_FUNC_ADDR(retro_cheat_reset);
    LOG_DEBUG_FUNC_ADDR(retro_cheat_set);
    LOG_DEBUG_FUNC_ADDR(retro_load_game);
    LOG_DEBUG_FUNC_ADDR(retro_load_game_special);
    LOG_DEBUG_FUNC_ADDR(retro_unload_game);
    LOG_DEBUG_FUNC_ADDR(retro_get_region);
    LOG_DEBUG_FUNC_ADDR(retro_get_memory_data);
    LOG_DEBUG_FUNC_ADDR(retro_get_memory_size);
    LOG_DEBUG_FUNC_ADDR(retro_set_environment);
    LOG_DEBUG_FUNC_ADDR(retro_set_video_refresh);
    LOG_DEBUG_FUNC_ADDR(retro_set_audio_sample);
    LOG_DEBUG_FUNC_ADDR(retro_set_audio_sample_batch);
    LOG_DEBUG_FUNC_ADDR(retro_set_input_poll);
    LOG_DEBUG_FUNC_ADDR(retro_set_input_state);
}

RetroModule::~RetroModule()
{
    LogFunctionName;
    sceKernelUnloadModule(_id, 0, NULL);
}