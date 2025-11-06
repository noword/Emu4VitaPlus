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

#define NEON
#ifdef NEON
#include "arm_neon.h"
#endif

#define MIN_STATE_RATE 5
#define NEXT_STATE_PERIOD 50000
#define THRESHOLD_RATE 0.1

#define DIFF_STEP 0x10
#define MEMCMP2(SIZE) memcmp_##SIZE
#define MEMCMP(SIZE) MEMCMP2(SIZE)
#define MEMCMP_DIFF_STEP MEMCMP(DIFF_STEP)

static inline int memcmp_0x10(const void *src, const void *dst)
#ifdef NEON
{
    // return 0 if same, 1 if different
    uint8x16_t s = vld1q_u8((const uint8_t *)src);
    uint8x16_t d = vld1q_u8((const uint8_t *)dst);
    uint8x16_t cmp = veorq_u8(s, d);
    return vgetq_lane_u64(vreinterpretq_u64_u8(cmp), 0) != 0LL || vgetq_lane_u64(vreinterpretq_u64_u8(cmp), 1) != 0LL;
}
#else
{
    const uint32_t *s = (const uint32_t *)src;
    const uint32_t *d = (const uint32_t *)dst;
    return s[0] != d[0] || s[1] != d[1] || s[2] != d[2] || s[3] != d[3];
}
#endif

RewindManager::RewindManager()
    : ThreadBase(_RewindThread),
      _contens(nullptr),
      _block_count(0),
      _tmp_buf(nullptr),
      _last_full_block(nullptr)
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
    _full_content_size = ALIGN_UP_10H(sizeof(RewindFullContent) + _state_size);

    size_t buf_size = gConfig->rewind_buf_size << 20;
    if (buf_size < _full_content_size * MIN_STATE_RATE)
    {
        size_t min_size = (_full_content_size * MIN_STATE_RATE) >> 20;
        LogError("the buffer size is too small, minimum required is %dMB", min_size);
        return false;
    }

    _threshold_size = _state_size * THRESHOLD_RATE;
    _tmp_buf = new uint8_t[_state_size];
    _contens = new RewindContens(buf_size);
    _delay.SetInterval(gEmulator->GetMsPerFrame());

    Start();
    LogDebug("  _contens.size: %08x _state_size: %08x _full_content_size:%08x _threshold_size:%08x",
             buf_size, _state_size, _full_content_size, _threshold_size);
    return true;
}

void RewindManager::Deinit()
{
    LogFunctionName;
    Stop(true);

    _blocks.Reset();
    if (_contens != nullptr)
    {
        delete _contens;
        _contens = nullptr;
    }

    if (_tmp_buf != nullptr)
    {
        delete[] _tmp_buf;
        _tmp_buf = nullptr;
    }
}

int RewindManager::_RewindThread(SceSize args, void *argp)
{
    CLASS_POINTER(RewindManager, rewind, argp);

    while (gEmulator->GetFrameCount() < 100 && rewind->IsRunning())
    {
        sceKernelDelayThread(20000);
    }

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
    if (_last_full_block == nullptr || _contens->GetDistance((uint8_t *)_last_full_block->content) * 2 > _contens->GetSize())
    {
        _SaveFullState(_blocks.Next(), false);
    }
    else if (_Serialize(_tmp_buf, _state_size))
    {
        if (!_SaveDiffState(_blocks.Next()))
        {
            _SaveFullState(_blocks.Current(), true);
        }
    }
}

void *RewindManager::_GetState()
{
    RewindBlock *block = _blocks.Prev(false);
    if (!block->IsValid())
    {
        return nullptr;
    }

    if (block->type == BLOCK_FULL)
    {
        // LogDebug("BLOCK_FULL %x %x", block, block->content);
        _blocks.Prev();
        return ((RewindFullContent *)block->content)->buf;
    }

    const RewindDiffContent *diff = (RewindDiffContent *)block->content;
    if (!diff->full_block->IsValid())
    {
        return nullptr;
    }
    // LogDebug("BLOCK_DIFF %x %x", block, diff);
    _blocks.Prev();
    const uint8_t *buf = diff->GetBuf();
    const DiffArea *area = diff->areas;
    _last_full_block = diff->full_block;
    memcpy(_tmp_buf, ((RewindFullContent *)_last_full_block->content)->buf, _state_size);
    for (size_t i = 0; i < diff->num; i++)
    {
        memcpy(_tmp_buf + area->offset, buf, area->size);
        buf += area->size;
        area++;
    }
    return _tmp_buf;
}

void RewindManager::_Rewind()
{
    if (_last_full_block != nullptr)
    {
        void *data = _GetState();

        if (data == nullptr)
        {
            _last_full_block = nullptr;
        }
        else
        {
            _UnSerialize(data, _state_size);
        }
    }

    Signal();
}

bool RewindManager::_SaveDiffState(RewindBlock *block)
{
    // LogFunctionName;
    if (!_Serialize(_tmp_buf, _state_size))
    {
        return false;
    }

    uint8_t *old_state = ((RewindFullContent *)(_last_full_block->content))->buf;
    uint8_t *new_state = _tmp_buf;
    bool last_diff = false;
    size_t offset = 0;
    size_t diff_size = sizeof(RewindDiffContent);
    uint32_t tail_size = _state_size % DIFF_STEP;
    size_t chunk_size = _state_size - tail_size;

    RewindDiffContent *content = (RewindDiffContent *)_contens->WriteBegin(_threshold_size);
    DiffArea *areas = content->areas;
    content->num = 0;

    for (; offset < chunk_size; offset += DIFF_STEP)
    {
        if (MEMCMP_DIFF_STEP(old_state + offset, new_state + offset) == 0)
        {
            if (last_diff)
            {
                last_diff = false;
                areas[content->num].size = offset - areas[content->num].offset;
                diff_size += sizeof(DiffArea) + areas[content->num].size;
                content->num++;
                if (diff_size > _threshold_size)
                {
                    return false;
                }
            }
        }
        else
        {
            if (!last_diff)
            {
                last_diff = true;
                areas[content->num].offset = offset;
            }
        }
    }

    if (tail_size > 0 && memcmp(old_state + offset, new_state + offset, tail_size) != 0)
    {
        if (last_diff)
        {
            areas[content->num].size = _state_size - areas[content->num].offset;
        }
        else
        {
            areas[content->num].offset = offset;
            areas[content->num].size = tail_size;
        }

        diff_size += sizeof(DiffArea) + areas[content->num].size;
        content->num++;
    }
    else if (last_diff)
    {
        areas[content->num].size = offset - areas[content->num].offset;
        diff_size += sizeof(DiffArea) + areas[content->num].size;
        content->num++;
    }

    if (diff_size > _threshold_size)
    {
        return false;
    }

    block->type = BLOCK_DIFF;
    block->index = _block_count;
    block->content = content;
    block->size = ALIGN_UP_10H(diff_size);

    content->magic = REWIND_BLOCK_MAGIC;
    content->index = _block_count++;
    content->full_block = _last_full_block;

    _contens->WriteEnd(block->size);

    uint8_t *buf = content->GetBuf();
    for (size_t i = 0; i < content->num; i++)
    {
        memcpy(buf, _tmp_buf + areas[i].offset, areas[i].size);
        buf += areas[i].size;
    }

    // LogDebug("  _SaveDiffState %08x %08x %08x", block->content, block->size, content->full_block->content);

    return true;
}

bool RewindManager::_SaveFullState(RewindBlock *block, bool from_tmp)
{
    // LogFunctionName;

    RewindFullContent *content = (RewindFullContent *)_contens->WriteBegin(_full_content_size);

    bool result = true;
    if (from_tmp)
    {
        memcpy(content->buf, _tmp_buf, _state_size);
    }
    else
    {
        result = _Serialize(content->buf, _state_size);
    }

    if (result)
    {
        content->magic = REWIND_BLOCK_MAGIC;
        content->index = _block_count;

        _last_full_block = block;

        block->type = BLOCK_FULL;
        block->index = _block_count++;
        block->size = _full_content_size;
        block->content = content;

        _contens->WriteEnd(_full_content_size);

        // LogDebug("  _SaveFullState %08x", block->content);
    }
    else
    {
        LogDebug("  _SaveFullState failed");
    }

    return result;
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