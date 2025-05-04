#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <SimpleIni.h>
#include "thread_base.h"
#include "delay.h"

struct Cheat
{
    uint32_t enable = 0;
    uint32_t cheat_type = 1;
    uint32_t handler = 0;
    std::string desc;
    std::string code;
    uint32_t big_endian = 0;
    uint32_t address;
    uint32_t value;
    uint32_t address_bit_position;
    uint32_t repeat_count = 1;
    uint32_t repeat_add_to_address = 1;
    uint32_t repeat_add_to_value;
    uint32_t memory_search_size;

    bool Load(CSimpleIniA *ini, size_t index);
    void Apply(int index, bool *run_cheat);
    static void CleanMemory();

private:
    void _ApplyRetro(bool *run_cheat);
    bool _SetupRetroCheatMeta(uint32_t *bytes_per_item, uint32_t *bits, uint32_t *mask);
    uint32_t _GetCurrentValue(uint32_t bytes_per_item);
    void _SetCurrentValue(uint32_t address, uint32_t bytes_per_item, uint32_t bits, uint32_t value);

    static uint8_t *_memory_data;
    static size_t _memory_size;
};

class Cheats : public std::vector<Cheat>, public ThreadBase
{
public:
    Cheats();
    virtual ~Cheats();

    bool Load(const char *path);

private:
    static int _CheatThread(SceSize args, void *argp);

    Delay<double> _delay;
};
