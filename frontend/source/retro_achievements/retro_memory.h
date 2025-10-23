#pragma once
#include <rc_libretro.h>

class RetroMemory
{
public:
    RetroMemory(const retro_memory_map *mmap, uint32_t console_id);
    virtual ~RetroMemory();

    uint32_t ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes);

private:
    rc_libretro_memory_regions_t _regions;
};