#pragma once
#include <stdint.h>
#include <string.h>
#include "thread_base.h"
#include "utils.h"
#include "delay.h"

#define REWIND_BLOCK_MAGIC 0x44574552

//   State[2]
//     |
//    xor
//     |
//  DiffBlock

class StateBuf
{
public:
    StateBuf(size_t size) : _index(0)
    {
        _size = ALIGN_UP_10H(size);
        _bufs[0] = new uint8_t[_size * 2];
        _bufs[1] = _bufs[0] + _size;
        memset(_bufs[0], 0, _size * 2);
    };

    virtual ~StateBuf()
    {
        delete[] _bufs[0];
    };

    uint8_t *Current() const { return _bufs[_index]; };
    uint8_t *Next() const { return _bufs[_index ^ 1]; };
    const uint8_t *First() { return _bufs[0]; };
    const uint8_t *Second() { return _bufs[1]; };
    int Index() const { return _index; };
    void Toggle() { _index ^= 1; };
    size_t Size() const { return _size; };

private:
    uint8_t *_bufs[2];
    int _index;
    size_t _size;
};

struct DiffArea
{
    uint32_t offset;
    uint32_t size;
    uint8_t diffs[]; // xor bytes
};

struct DiffBlock
{
    uint32_t magic; // REWIND_BLOCK_MAGIC
    DiffBlock *prev;
    uint32_t count;
    uint32_t num; // num of areas
    DiffArea areas[];

    bool IsValid(uint32_t excepted) { return magic == REWIND_BLOCK_MAGIC && count == excepted; }
    bool IsValid() { return magic == REWIND_BLOCK_MAGIC; };
    void Invalidate() { magic = 0; };
};

class DiffBuf // it's a ring buffer, store DiffBlock
{
public:
    DiffBuf(size_t size, size_t tail_size) : _size(size)
    {
        _buf = new uint8_t[size + ALIGN_UP_10H(tail_size)];
        Reset();
    };

    virtual ~DiffBuf() { delete[] _buf; };

    void Reset()
    {
        memset(_buf, 0, sizeof(DiffBlock)); // only clean the first block
        _current = 0;
    };

    DiffBlock *Current() { return (DiffBlock *)(_buf + _current); };

    void Increase(size_t size, uint32_t count)
    {
        DiffBlock *prev = Current();

        _current += size;
        if (_current > _size)
            _current = 0;

        DiffBlock *block = Current();
        block->magic = 0;
        block->prev = prev;
        block->count = count;
    };

    void Rewind()
    {
        DiffBlock *block = Current();
        if (block->prev)
        {
            _current = (uint8_t *)(block->prev) - _buf;
            if (!Current()->IsValid(block->count - 1))
            {
                Current()->Invalidate();
            }
        }
        else
        {
            block->Invalidate();
        }
    }

private:
    size_t _size;
    uint8_t *_buf;
    size_t _current;
};

class RewindManager : public ThreadBase
{
public:
    RewindManager();
    virtual ~RewindManager();

    bool Init();
    void Deinit();

private:
    static int _RewindThread(SceSize args, void *argp);
    void _SaveState();
    void *_GetState();
    void _Rewind();

    bool _Serialize(void *data, size_t size);
    bool _UnSerialize(void *data, size_t size);

    StateBuf *_state;
    DiffBuf *_diff;
    Delay<double> _delay;
    uint32_t _count;
    size_t _state_size;
};