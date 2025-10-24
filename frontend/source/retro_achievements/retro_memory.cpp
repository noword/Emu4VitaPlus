#include <libretro.h>
#include "retro_memory.h"
#include "log.h"

static void GetCoreMemoryInfo(uint32_t id, rc_libretro_core_memory_info_t *info)
{
    info->size = retro_get_memory_size(id);
    info->data = (uint8_t *)retro_get_memory_data(id);
}

RetroMemory::RetroMemory(const retro_memory_map *mmap, uint32_t console_id)
{
    rc_libretro_memory_init(&_regions, mmap, GetCoreMemoryInfo, console_id);
}

RetroMemory::~RetroMemory()
{
    rc_libretro_memory_destroy(&_regions);
}

uint32_t RetroMemory::Read(uint32_t address, uint8_t *buffer, uint32_t num_bytes)
{
    return rc_libretro_memory_read(&_regions, address, buffer, num_bytes);
}