#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include <libretro.h>
#include <string.h>

#define PRINT_VALUE(FUNC) sceClibPrintf(#FUNC " %08x\n", FUNC);
#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffU)

unsigned int sceUserMainThreadStackSize __attribute__((used)) = 0x100000;
unsigned int sceLibcHeapExtendedAlloc __attribute__((used)) = 1;
unsigned int sceLibcHeapSize __attribute__((used)) = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
const char sceUserMainThreadName[] = "retro";
const int sceKernelPreloadModuleInhibit = SCE_KERNEL_PRELOAD_INHIBIT_LIBC |
                                          SCE_KERNEL_PRELOAD_INHIBIT_LIBFIOS2 |
                                          SCE_KERNEL_PRELOAD_INHIBIT_APPUTIL |
                                          SCE_KERNEL_PRELOAD_INHIBIT_LIBSCEFT2;

extern unsigned _newlib_heap_size;
extern char *_newlib_heap_base, *_newlib_heap_end, *_newlib_heap_cur;
extern SceKernelLwMutexWork _newlib_sbrk_mutex;

extern int _newlib_vm_memblock;
extern int _newlib_vm_size;

extern void _init_vita_heap(void);
extern void _free_vita_heap(void);
extern void __libc_init_array(void);
extern void __libc_fini_array();

struct Heap
{
    unsigned _newlib_heap_size;
    char *_newlib_heap_base;
    char *_newlib_heap_end;
    char *_newlib_heap_cur;
    SceKernelLwMutexWork _newlib_sbrk_mutex;
};

int module_stop(SceSize argc, const void *args)
{
    return SCE_KERNEL_STOP_SUCCESS;
}

int module_exit()
{
    __libc_fini_array();
    _free_vita_heap();
    return SCE_KERNEL_STOP_SUCCESS;
}

int main(SceSize, void *) __attribute__((weak, alias("module_start")));
int _start(SceSize, void *) __attribute__((weak, alias("module_start")));

int module_start(SceSize argc, void *args)
{
    struct Heap *heap = *(struct Heap **)args;

    _newlib_heap_size = heap->_newlib_heap_size;
    _newlib_heap_base = heap->_newlib_heap_base;
    _newlib_heap_end = heap->_newlib_heap_end;
    _newlib_heap_cur = heap->_newlib_heap_cur;
    _newlib_sbrk_mutex = heap->_newlib_sbrk_mutex;

    _init_vita_heap();
    __libc_init_array();

    PRINT_VALUE(_newlib_heap_size);
    PRINT_VALUE(_newlib_heap_base);
    PRINT_VALUE(_newlib_heap_end);
    PRINT_VALUE(_newlib_heap_cur);

    PRINT_VALUE(_newlib_vm_memblock);
    PRINT_VALUE(_newlib_vm_size);

    PRINT_VALUE(module_stop);
    PRINT_VALUE(module_exit);
    PRINT_VALUE(sceUserMainThreadName);
    PRINT_VALUE(sceKernelPreloadModuleInhibit);
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

    return SCE_KERNEL_START_SUCCESS;
}
