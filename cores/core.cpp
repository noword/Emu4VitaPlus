#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <stdio.h>
#include <libretro.h>

#define PRINT_VALUE(FUNC) printf(#FUNC " %08x\n", FUNC);
#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffU)

unsigned int sceUserMainThreadStackSize __attribute__((used)) = 0x100000;
unsigned int sceLibcHeapExtendedAlloc __attribute__((used)) = 1;
unsigned int sceLibcHeapSize __attribute__((used)) = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;

#ifdef __cplusplus
extern "C"
{
#endif

    extern void _init_vita_heap(void);
    extern void _free_vita_heap(void);

    int module_stop(SceSize argc, const void *args)
    {
        return SCE_KERNEL_STOP_SUCCESS;
    }

    int module_exit()
    {
        _free_vita_heap();
        return SCE_KERNEL_STOP_SUCCESS;
    }

    int _start(SceSize, void *) __attribute__((weak, alias("module_start")));

    int module_start(SceSize argc, void *args)
    {
        PRINT_VALUE(module_stop);
        PRINT_VALUE(module_exit);
        PRINT_VALUE(sceUserMainThreadStackSize);
        PRINT_VALUE(sceLibcHeapExtendedAlloc);
        PRINT_VALUE(sceLibcHeapSize);
        PRINT_VALUE(retro_init);
        PRINT_VALUE(retro_deinit);
        PRINT_VALUE(retro_api_version);
        PRINT_VALUE(retro_get_system_info);
        PRINT_VALUE(retro_get_system_av_info);
        PRINT_VALUE(retro_set_controller_port_device);
        PRINT_VALUE(retro_reset);
        PRINT_VALUE(retro_run);
        PRINT_VALUE(retro_serialize_size);
        PRINT_VALUE(retro_serialize);
        PRINT_VALUE(retro_unserialize);
        PRINT_VALUE(retro_cheat_reset);
        PRINT_VALUE(retro_cheat_set);
        PRINT_VALUE(retro_load_game);
        PRINT_VALUE(retro_load_game_special);
        PRINT_VALUE(retro_unload_game);
        PRINT_VALUE(retro_get_region);
        PRINT_VALUE(retro_get_memory_data);
        PRINT_VALUE(retro_get_memory_size);
        PRINT_VALUE(retro_set_environment);
        PRINT_VALUE(retro_set_video_refresh);
        PRINT_VALUE(retro_set_audio_sample);
        PRINT_VALUE(retro_set_audio_sample_batch);
        PRINT_VALUE(retro_set_input_poll);
        PRINT_VALUE(retro_set_input_state);

        _init_vita_heap();

        //         RetroCore *retro = *(RetroCore **)args;

        // #define ASSIGN_FUNC(FUNC) retro->FUNC = FUNC;

        //         ASSIGN_FUNC(retro_init);
        //         ASSIGN_FUNC(retro_deinit);
        //         ASSIGN_FUNC(retro_api_version);
        //         ASSIGN_FUNC(retro_get_system_info);
        //         ASSIGN_FUNC(retro_get_system_av_info);
        //         ASSIGN_FUNC(retro_set_controller_port_device);
        //         ASSIGN_FUNC(retro_reset);
        //         ASSIGN_FUNC(retro_run);
        //         ASSIGN_FUNC(retro_serialize_size);
        //         ASSIGN_FUNC(retro_serialize);
        //         ASSIGN_FUNC(retro_unserialize);
        //         ASSIGN_FUNC(retro_cheat_reset);
        //         ASSIGN_FUNC(retro_cheat_set);
        //         ASSIGN_FUNC(retro_load_game);
        //         ASSIGN_FUNC(retro_load_game_special);
        //         ASSIGN_FUNC(retro_unload_game);
        //         ASSIGN_FUNC(retro_get_region);
        //         ASSIGN_FUNC(retro_get_memory_data);
        //         ASSIGN_FUNC(retro_get_memory_size);
        //         ASSIGN_FUNC(retro_set_environment);
        //         ASSIGN_FUNC(retro_set_video_refresh);
        //         ASSIGN_FUNC(retro_set_audio_sample);
        //         ASSIGN_FUNC(retro_set_audio_sample_batch);
        //         ASSIGN_FUNC(retro_set_input_poll);
        //         ASSIGN_FUNC(retro_set_input_state);

        return SCE_KERNEL_START_SUCCESS;
    }
#ifdef __cplusplus
}
#endif