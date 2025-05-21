#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <libretro.h>
#include <string.h>
#include "retro_module.h"
#include "file.h"
#include "log.h"

#define LOG_DEBUG_VALUE(FUNC) LogInfo("%08x " #FUNC, FUNC);

extern "C"
{
    extern char *_newlib_heap_base, *_newlib_heap_end, *_newlib_heap_cur;
    extern SceKernelLwMutexWork _newlib_sbrk_mutex;
};

struct Heap
{
    char **_newlib_heap_base;
    char **_newlib_heap_end;
    char **_newlib_heap_cur;
    SceKernelLwMutexWork *_newlib_sbrk_mutex;
};

RetroModule::RetroModule(const char *name)
{
    LogFunctionName;

    LOG_DEBUG_VALUE(_newlib_heap_base);
    LOG_DEBUG_VALUE(_newlib_heap_end);
    LOG_DEBUG_VALUE(_newlib_heap_cur);
    LogInfo("_newlib_heap_size: %d", _newlib_heap_end - _newlib_heap_base);

    char module[SCE_FIOS_PATH_MAX];

    Heap heap{&_newlib_heap_base,
              &_newlib_heap_end,
              &_newlib_heap_cur,
              &_newlib_sbrk_mutex};

    Heap *p = &heap;
    snprintf(module, SCE_FIOS_PATH_MAX, "app0:modules/%s.suprx", name);
    _id = sceKernelLoadStartModule(module, sizeof(Heap *), &p, 0, NULL, NULL);
    if (_id < 0)
    {
        LogError("Failed to load module %s: %d", module, _id);
        return;
    }

    LOG_DEBUG_VALUE(retro_init);
    LOG_DEBUG_VALUE(retro_deinit);
    LOG_DEBUG_VALUE(retro_api_version);
    LOG_DEBUG_VALUE(retro_get_system_info);
    LOG_DEBUG_VALUE(retro_get_system_av_info);
    LOG_DEBUG_VALUE(retro_set_controller_port_device);
    LOG_DEBUG_VALUE(retro_reset);
    LOG_DEBUG_VALUE(retro_run);
    LOG_DEBUG_VALUE(retro_serialize_size);
    LOG_DEBUG_VALUE(retro_serialize);
    LOG_DEBUG_VALUE(retro_unserialize);
    LOG_DEBUG_VALUE(retro_cheat_reset);
    LOG_DEBUG_VALUE(retro_cheat_set);
    LOG_DEBUG_VALUE(retro_load_game);
    LOG_DEBUG_VALUE(retro_load_game_special);
    LOG_DEBUG_VALUE(retro_unload_game);
    LOG_DEBUG_VALUE(retro_get_region);
    LOG_DEBUG_VALUE(retro_get_memory_data);
    LOG_DEBUG_VALUE(retro_get_memory_size);
    LOG_DEBUG_VALUE(retro_set_environment);
    LOG_DEBUG_VALUE(retro_set_video_refresh);
    LOG_DEBUG_VALUE(retro_set_audio_sample);
    LOG_DEBUG_VALUE(retro_set_audio_sample_batch);
    LOG_DEBUG_VALUE(retro_set_input_poll);
    LOG_DEBUG_VALUE(retro_set_input_state);
}

RetroModule::~RetroModule()
{
    LogFunctionName;
    sceKernelUnloadModule(_id, 0, NULL);
}