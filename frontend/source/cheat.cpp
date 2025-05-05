#include <SimpleIni.h>
#include <stdlib.h>
#include "cheat.h"
#include "utils.h"
#include "log.h"
#include "emulator.h"
#include "retro_module.h"
#include "app.h"

#define KEY_BUF_SIZE 32
#define KEY(K) snprintf(key, KEY_BUF_SIZE, "cheat%d_" K, index);

enum
{
    CHEAT_HANDLER_TYPE_EMU = 0,
    CHEAT_HANDLER_TYPE_RETRO,
    CHEAT_HANDLER_TYPE_END
};

enum
{
    CHEAT_TYPE_DISABLED = 0,
    CHEAT_TYPE_SET_TO_VALUE,
    CHEAT_TYPE_INCREASE_VALUE,
    CHEAT_TYPE_DECREASE_VALUE,
    CHEAT_TYPE_RUN_NEXT_IF_EQ,
    CHEAT_TYPE_RUN_NEXT_IF_NEQ,
    CHEAT_TYPE_RUN_NEXT_IF_LT,
    CHEAT_TYPE_RUN_NEXT_IF_GT
};

uint8_t *Cheat::_memory_data = nullptr;
size_t Cheat::_memory_size = 0;

static std::string GetValue(CSimpleIniA *ini, const char *key)
{
    std::string s(ini->GetValue("", key, ""));
    Utils::StripQuotes(&s);
    return s;
}

static long GetLong(CSimpleIniA *ini, const char *key)
{
    std::string s = GetValue(ini, key);
    return strtol(s.c_str(), NULL, 10);
}

static std::string GetValue(CSimpleIniA *ini, const char *key, size_t index)
{
    char k[KEY_BUF_SIZE];
    snprintf(k, KEY_BUF_SIZE, "cheat%d_%s", index, key);
    return GetValue(ini, k);
}

static long GetLong(CSimpleIniA *ini, const char *key, size_t index)
{
    std::string s = GetValue(ini, key, index);
    return strtol(s.c_str(), NULL, 10);
}

static bool GetBool(CSimpleIniA *ini, const char *key, size_t index)
{
    std::string s = GetValue(ini, key, index);
    return s == "true";
}

#define GET_VALUE(K) K = GetValue(ini, #K, index)
#define GET_BOOL(K) K = GetBool(ini, #K, index)
#define GET_LONG(K)                       \
    {                                     \
        long V = GetLong(ini, #K, index); \
        if (V != 0)                       \
            K = V;                        \
    }

bool Cheat::Load(CSimpleIniA *ini, size_t index)
{
    char key[KEY_BUF_SIZE];
    GET_VALUE(desc);
    // LogDebug("desc %d %s", index, desc.c_str());
    if (desc.size() == 0)
    {
        return false;
    }

    GET_VALUE(code);
    GET_BOOL(enable);
    GET_BOOL(big_endian);
    GET_LONG(handler);
    GET_LONG(cheat_type);
    GET_LONG(address);
    GET_LONG(value);
    GET_LONG(address_bit_position);
    GET_LONG(repeat_count);
    GET_LONG(repeat_add_to_address);
    GET_LONG(repeat_add_to_value);
    GET_LONG(memory_search_size);

    return true;
}

void Cheat::Apply(int index, bool *run_cheat)
{
    if (code.size() > 0)
    {
        gRetro->retro_cheat_set(index, 1, code.c_str());
        return;
    }
    else
    {
        _ApplyRetro(run_cheat);
    }
}

void Cheat::CleanMemory()
{
    _memory_data = nullptr;
    _memory_size = 0;
}

bool Cheat::_SetupRetroCheatMeta(uint32_t *bytes_per_item, uint32_t *bits, uint32_t *mask)
{
    switch (memory_search_size)
    {
    case 0:
        *bytes_per_item = 1;
        *bits = 1;
        *mask = 1;
        break;

    case 1:
        *bytes_per_item = 1;
        *bits = 2;
        *mask = 2;
        break;

    case 2:
        *bytes_per_item = 1;
        *bits = 4;
        *mask = 0xf;
        break;

    case 3:
        *bytes_per_item = 1;
        *bits = 8;
        *mask = 0xff;
        break;

    case 4:
        *bytes_per_item = 2;
        *bits = 8;
        *mask = 0xffff;
        break;

    case 5:
        *bytes_per_item = 4;
        *bits = 8;
        *mask = 0xffffffff;
        break;

    default:
        // AppLog("wrong value of memory_search_size: %d", bitsize);
        return false;
    }

    return true;
}

uint32_t Cheat::_GetCurrentValue(uint32_t bytes_per_item)
{
    uint8_t *curr = _memory_data + address;
    uint32_t val;
    switch (bytes_per_item)
    {
    case 2:
        val = big_endian ? (curr[0] << 8) | curr[1] : curr[0] | (curr[1] << 8);
        break;
    case 4:
        val = big_endian ? (curr[0] << 24) | (curr[1] << 16) | (curr[2] << 8) | curr[3] : curr[0] | (curr[1] << 8) | (curr[2] << 16) | curr[3] << 24;
        break;
    case 1:
    default:
        val = *curr;
        break;
    }
    return val;
}

void Cheat::_SetCurrentValue(uint32_t address, uint32_t bytes_per_item, uint32_t bits, uint32_t val)
{
    uint8_t *curr = _memory_data + address;
    switch (bytes_per_item)
    {
    case 2:
        if (big_endian)
        {
            curr[0] = (val >> 8) & 0xff;
            curr[1] = val & 0xff;
        }
        else
        {
            curr[0] = val & 0xff;
            curr[1] = (val >> 8) & 0xff;
        }
        break;

    case 4:
        if (big_endian)
        {
            curr[0] = (val >> 24) & 0xff;
            curr[1] = (val >> 16) & 0xff;
            curr[2] = (val >> 8) & 0xff;
            curr[3] = val & 0xff;
        }
        else
        {
            curr[0] = val & 0xff;
            curr[1] = (val >> 8) & 0xff;
            curr[2] = (val >> 16) & 0xff;
            curr[3] = (val >> 24) & 0xff;
        }
        break;

    case 1:
        if (bits < 8)
        {
            uint8_t mask;
            uint8_t v = curr[0];
            for (int i = 0; i < 8; i++)
            {
                if ((address_bit_position >> i) & 1)
                {
                    mask = ~((1 << i) & 0xff);
                    v &= mask;
                    v |= ((val >> i) & 1) << i;
                }
            }
            curr[0] = v;
        }
        else
            curr[0] = val & 0xff;
        break;

    default:
        curr[0] = val & 0xff;
        break;
    }
}

void Cheat::_ApplyRetro(bool *run_cheat)
{
    if (!(*run_cheat))
    {
        *run_cheat = true;
        return;
    }

    if (_memory_data == nullptr)
    {
        _memory_data = (uint8_t *)gRetro->retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
        _memory_size = gRetro->retro_get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
        LogDebug("_memory_data:%08x _memory_size:%08x\n", _memory_data, _memory_size);
    }

    if (_memory_data == nullptr || _memory_size == 0)
    {
        return;
    }

    uint32_t bytes_per_item;
    uint32_t bits;
    uint32_t mask;

    if (!_SetupRetroCheatMeta(&bytes_per_item, &bits, &mask))
    {
        return;
    }

    uint32_t current_value = _GetCurrentValue(bytes_per_item);
    bool set_value = false;

    switch (cheat_type)
    {
    case CHEAT_TYPE_SET_TO_VALUE:
        current_value = value;
        set_value = true;
        break;

    case CHEAT_TYPE_INCREASE_VALUE:
        current_value += value;
        set_value = true;
        break;

    case CHEAT_TYPE_DECREASE_VALUE:
        current_value -= value;
        set_value = true;
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_EQ:
        *run_cheat = (value == current_value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_NEQ:
        *run_cheat = (value != current_value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_LT:
        *run_cheat = (value < current_value);
        break;

    case CHEAT_TYPE_RUN_NEXT_IF_GT:
        *run_cheat = (value > current_value);
        break;

    default:
        // AppLog("warning: wrong cheat type: %d\n", data->cheat_type);
        break;
    }

    if (set_value)
    {
        uint32_t addr = address;
        for (int i = 0; i < repeat_count; i++)
        {
            _SetCurrentValue(addr, bytes_per_item, bits, current_value);
            current_value += repeat_add_to_value;
            current_value %= mask;

            if (bits < 8)
            {
                int address_mask = address_bit_position;
                for (int i = 0; i < repeat_add_to_address; i++)
                {
                    address_mask = (address_mask << mask) & 0xff;
                    if (address_mask == 0)
                    {
                        address_mask = mask;
                        address++;
                    }
                }
            }
            else
            {
                addr += repeat_add_to_address * bytes_per_item;
            }
            addr %= _memory_size;
        }
    }
}

Cheats::Cheats() : ThreadBase(_CheatThread)
{
}

Cheats::~Cheats()
{
}

bool Cheats::Load(const char *path)
{
    LogFunctionName;

    this->clear();

    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        LogDebug("load cheat failed: %s", path);
        return false;
    }

    int size = GetLong(&ini, "cheats");
    if (size > 0)
    {
        this->reserve(size);
    }

    int count = 0;
    while (true)
    {
        Cheat cheat;
        if (cheat.Load(&ini, count))
        {
            LogDebug("  cheat %d %s", count, cheat.desc.c_str());
            this->emplace_back(cheat);
        }
        else
        {
            break;
        }

        count++;
    }

    return this->size() > 0;
}

int Cheats::_CheatThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(Cheats, cheats, argp);

    while (gEmulator->GetFrameCount() < 100 && cheats->IsRunning())
    {
        sceKernelDelayThread(20000);
    }

    cheats->_delay.SetInterval(gEmulator->GetMsPerFrame());
    Cheat::CleanMemory();

    while (cheats->IsRunning() && (gStatus.Get() & (APP_STATUS_RUN_GAME | APP_STATUS_SHOW_UI_IN_GAME)) != 0)
    {
        bool applied = false;
        int index = 0;
        bool run_cheat = true;
        for (Cheat &cheat : *cheats)
        {
            if (cheat.enable)
            {
                cheat.Apply(index, &run_cheat);
                applied = true;
            }
            index++;
        }

        if (applied)
        {
            cheats->_delay.Wait();
        }
        else
        {
            cheats->Wait();
        }
    }

    LogDebug("Cheat thread done");
    return 0;
}