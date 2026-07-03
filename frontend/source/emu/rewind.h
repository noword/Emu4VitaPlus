#pragma once
#include <stdint.h>
#include <string.h>
#include "thread_base.h"
#include "utils.h"
#include "delay.h"

#define DEFAULT_BLOCK_SIZE 0x400
//"REWD"
#define REWIND_BLOCK_MAGIC 0x44574552

class StateBuf
{
public:
    StateBuf(size_t size) : _index(0)
    {
        size = ALIGN_UP_10H(size);
        _bufs[0] = new uint8_t[size * 2];
        _bufs[1] = _bufs[0] + size;
    };

    virtual ~StateBuf()
    {
        delete[] _bufs[0];
    };

    uint8_t *Current() { return _bufs[_index]; };
    uint8_t *Next() { return _bufs[_index ^ 1]; };
    void Toggle() { _index ^= 1; };

private:
    uint8_t *_bufs[2];
    int _index;
};

struct DiffArea
{
    uint32_t offset;
    uint32_t size;
};

struct DiffContent
{
    uint32_t magic;
    uint32_t index;
    DiffArea areas[];
};

struct DiffBlock
{
    uint32_t index;
    uint32_t num;
    DiffContent *content;

    bool IsValid()
    {
        return content && content->magic == REWIND_BLOCK_MAGIC && content->index == index;
    }
};

template <size_t BLOCK_SIZE>
class DiffBlocks
{
    static_assert(BLOCK_SIZE > 0 && (BLOCK_SIZE & (BLOCK_SIZE - 1)) == 0, "total must be a power of 2!");

public:
    DiffBlocks() :
    {
        _blocks = new DiffBlock[BLOCK_SIZE];
    };

    virtual ~DiffBlocks()
    {
        delete[] _blocks;
    };

    DiffBlock *Current() { return &_blocks[_current]; };

private:
    DiffBlock *_blocks;
    size_t _current;
    size_t _total;
};

//-------------------------------
enum BlockType
{
    BLOCK_FULL,
    BLOCK_DIFF
};

struct RewindContent
{
    uint32_t magic;
    uint32_t index;
};

struct RewindBlock
{
    BlockType type;
    uint32_t index;
    RewindContent *content;
    uint32_t size;

    bool IsValid()
    {
        return content && content->magic == REWIND_BLOCK_MAGIC && content->index == index;
    }
};

struct RewindFullContent : RewindContent
{
    uint8_t buf[];
};

struct RewindDiffContent : RewindContent
{
    RewindBlock *full_block;
    uint32_t num;
    DiffArea areas[];

    uint8_t *GetBuf() const { return (uint8_t *)(this->areas) + this->num * sizeof(DiffArea); };
};

class RewindContens
{
public:
    RewindContens(size_t total_bytes)
        : _total_bytes(total_bytes),
          _current(0)
    {
        _data = new uint8_t[total_bytes];
    };

    virtual ~RewindContens()
    {
        delete[] _data;
    }

    uint8_t *GetData() { return _data; };

    uint8_t *WriteBegin(size_t max_size)
    {
        if (_current + max_size >= _total_bytes)
        {
            _current = 0;
        }
        return _data + _current;
    }

    void WriteEnd(size_t size)
    {
        _current += size;
    }

    size_t GetDistance(uint8_t *b)
    {
        size_t pos = b - _data;
        if (_current >= pos)
        {
            return _current - pos;
        }
        else
        {
            return _total_bytes + pos - _current;
        }
    }

    size_t GetSize()
    {
        return _total_bytes;
    }

private:
    uint8_t *_data;
    size_t _total_bytes;
    size_t _current;
};

class RewindBlocks
{
public:
    RewindBlocks(size_t total)
        : _total(total)
    {
        _blocks = new RewindBlock[total];
    };

    virtual ~RewindBlocks()
    {
        delete[] _blocks;
    };

    void Reset()
    {
        _current = _total;
        memset(_blocks, 0, _total * sizeof(RewindBlock));
    }

    RewindBlock *Current()
    {
        return _current == _total ? nullptr : (_blocks + _current);
    }

    RewindBlock *Next(bool move = true)
    {
        if (move)
        {
            LOOP_PLUS_ONE(_current, _total);
            return _blocks + _current;
        }
        else
        {
            size_t current = _current;
            LOOP_PLUS_ONE(current, _total);
            return _blocks + current;
        }
    };

    RewindBlock *Prev(bool move = true)
    {
        if (move)
        {
            LOOP_MINUS_ONE(_current, _total);
            return _blocks + _current;
        }
        else
        {
            size_t current = _current;
            LOOP_MINUS_ONE(current, _total);
            return _blocks + current;
        }
    };

private:
    RewindBlock *_blocks;
    size_t _total;
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

    bool _SaveFullState(RewindBlock *block, bool from_tmp = false);
    bool _SaveDiffState(RewindBlock *block);

    bool _Serialize(void *data, size_t size);
    bool _UnSerialize(void *data, size_t size);

    size_t _state_size;
    size_t _full_content_size;
    size_t _threshold_size;
    uint32_t _block_count;
    uint8_t *_tmp_buf;
    RewindBlock *_last_full_block;
    Delay<double> _delay;

    RewindBlocks _blocks{BLOCK_SIZE};
    RewindContens *_contens;
};