#include <algorithm>
#include "defines.h"
#include "log.h"
#include "config.h"
#include "rewind.h"
#include "utils.h"
#include "emulator.h"
#include "global.h"
#include "file.h"
#include "profiler.h"
#include "arm_neon.h"

#define MIN_STATE_RATE 5
#define NEXT_STATE_PERIOD 50000
#define THRESHOLD_RATE 0.1

#define DIFF_STEP 0x10

static inline bool MemCmpXor0x10(const void *__restrict src, const void *__restrict dst, void *__restrict xor_dst)
{
    uint8x16_t s = vld1q_u8((const uint8_t *)src);
    uint8x16_t d = vld1q_u8((const uint8_t *)dst);

    uint8x16_t x = veorq_u8(s, d);

    uint64x2_t x64 = vreinterpretq_u64_u8(x);
    uint64_t r = vgetq_lane_u64(x64, 0) | vgetq_lane_u64(x64, 1);

    if (r)
    {
        vst1q_u8((uint8_t *)xor_dst, x);
        return true;
    }

    return false;
}

static inline void MemXor(void *__restrict dst, const void *__restrict xor_data, size_t size)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *x = (const uint8_t *)xor_data;

    while (size >= 32)
    {
        uint8x16_t d0 = vld1q_u8(d);
        uint8x16_t d1 = vld1q_u8(d + 16);

        uint8x16_t x0 = vld1q_u8(x);
        uint8x16_t x1 = vld1q_u8(x + 16);

        vst1q_u8(d, veorq_u8(d0, x0));
        vst1q_u8(d + 16, veorq_u8(d1, x1));

        d += 32;
        x += 32;
        size -= 32;
    }

    while (size)
    {
        uint8x16_t vd = vld1q_u8(d);
        uint8x16_t vx = vld1q_u8(x);

        vst1q_u8(d, veorq_u8(vd, vx));

        d += 16;
        x += 16;
        size -= 16;
    }
}

RewindManager::RewindManager()
    : ThreadBase(_RewindThread, "rewind"),
      _state(nullptr),
      _diff(nullptr)
{
    LogFunctionName;
}

RewindManager::~RewindManager()
{
    LogFunctionName;
    _keep_running = false;
    Deinit();
}

bool RewindManager::Init()
{
    LogFunctionName;

    Deinit();

    _state_size = retro_serialize_size();
    _state = new StateBuf(_state_size);

    size_t buf_size = gConfig->rewind_buf_size << 20;
    _diff = new DiffBuf(buf_size, _state_size);

    _delay.SetInterval(gEmulator->GetMsPerFrame());

    _count = 1;

    Start();
    LogDebug("  _state_size: %08x diff buff size:%08x", _state_size, buf_size);
    return true;
}

void RewindManager::Deinit()
{
    LogFunctionName;
    Stop(true);

    if (_state != nullptr)
    {
        delete _state;
        _state = nullptr;
    }

    if (_diff != nullptr)
    {
        delete[] _diff;
        _diff = nullptr;
    }
}

int RewindManager::_RewindThread(SceSize args, void *argp)
{
    CLASS_POINTER(RewindManager, rewind, argp);

    while (gEmulator->GetFrameCount() < 100 && rewind->IsRunning())
    {
        sceKernelDelayThread(20000);
    }

    rewind->_Serialize(rewind->_state->Current(), rewind->_state->Size());

    while (rewind->IsRunning())
    {
        rewind->_delay.Wait();

        BeginProfile("RewindManager");
        switch (gStatus.Get())
        {
        case APP_STATUS_REWIND_GAME:
            rewind->_Rewind();
            break;
        case APP_STATUS_RUN_GAME:
            rewind->_SaveState();
            break;
        default:
            rewind->Wait();
            break;
        }
        EndProfile("RewindManager");
        // LogDebug("_RewindThread loop");
    }

    LogDebug("_RewindThread exit");
    return 0;
}

void RewindManager::_SaveState()
{
    _state->Toggle();
    _Serialize(_state->Current(), _state->Size());

    DiffBlock *diff_block = _diff->Current();
    diff_block->magic = REWIND_BLOCK_MAGIC;
    diff_block->count = _count++;
    diff_block->num = 0;

    const uint8_t *state0 = _state->First();
    const uint8_t *state1 = _state->Second();
    DiffArea *area = diff_block->areas;
    uint8_t *diffs = area->diffs;
    bool last_diff = false;
    for (int offset = 0; offset < _state->Size(); offset += DIFF_STEP)
    {
        if (MemCmpXor0x10(state0, state1, diffs))
        {
            diffs += DIFF_STEP;
            if (last_diff)
            {
                area->size += DIFF_STEP;
            }
            else
            {
                area->offset = offset;
                area->size = DIFF_STEP;
                diff_block->num++;
                last_diff = true;
            }
        }
        else
        {
            if (last_diff)
            {
                area = (DiffArea *)diffs;
            }
            last_diff = false;
        }

        state0 += DIFF_STEP;
        state1 += DIFF_STEP;
    }

    if (diff_block->num > 0)
    {
        _diff->Increase(diffs - (uint8_t *)area);
    }
}

void *RewindManager::_GetState()
{
    DiffBlock *diff_block = _diff->Current();
    if (!diff_block->IsValid())
        return nullptr;

    uint8_t *buf = _state->Current();
    DiffArea *area = diff_block->areas;
    for (uint32_t i = 0; i < diff_block->num; i++)
    {
        MemXor(buf + area->offset, area->diffs, area->size);
        area = (DiffArea *)((uint8_t *)area + sizeof(DiffArea) + area->size);
    }

    _diff->Rewind();

    return buf;
}

void RewindManager::_Rewind()
{
    void *data = _GetState();
    if (data != nullptr)
    {
        _UnSerialize(data, _state_size);
    }

    Signal();
}

bool RewindManager::_Serialize(void *data, size_t size)
{
    gEmulator->Lock();
    bool result = retro_serialize(data, size);
    gEmulator->Unlock();
    return result;
}

bool RewindManager::_UnSerialize(void *data, size_t size)
{
    gEmulator->Lock();
    bool result = retro_unserialize(data, size);
    gEmulator->Unlock();
    return result;
}