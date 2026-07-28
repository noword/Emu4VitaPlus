#pragma once
#include <atomic>
#include <string.h>
#include "utils.h"
#include "log.h"

template <typename T, size_t TOTAL_SIZE>
class CircleBuf
{
public:
    CircleBuf()
        : _read_pos(0),
          _write_pos(0),
          _tmp(nullptr),
          _tmp_size(0)
    {
        static_assert(IS_POWER_OF_TWO(TOTAL_SIZE), "buf size must be power of two");
        _buf = new T[TOTAL_SIZE];
    };

    virtual ~CircleBuf()
    {
        if (_tmp != nullptr)
        {
            delete[] _tmp;
        }
        delete[] _buf;
    };

    void Reset()
    {
        _read_pos = 0;
        _write_pos = 0;
        if (_tmp != nullptr)
        {
            delete[] _tmp;
            _tmp = nullptr;
            _tmp_size = 0;
        }
    };

    T *WriteBegin(size_t size, bool &continuous)
    {
        if (unlikely(size > FreeSize()))
        {
            return nullptr;
        }

        const size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        continuous = (write_pos + size) < TOTAL_SIZE;
        return continuous ? _buf + write_pos : _GetTmpBuf(size);
    };

    void WriteEnd(size_t size, bool continuous)
    {
        size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        if (likely(continuous))
        {
            write_pos += size;
            _write_pos.store(write_pos, std::memory_order_release);
        }
        else
        {
            Write(_tmp, size);
        }
    }

    bool Write(const T *data, size_t size)
    {
        if (unlikely(size > FreeSize()))
        {
            return false;
        }

        size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        if (likely((write_pos + size) < TOTAL_SIZE))
        {
            memcpy(_buf + write_pos, data, size * sizeof(T));
            write_pos += size;
        }
        else
        {
            size_t first_size = (TOTAL_SIZE - write_pos);
            size_t second_size = size - first_size;
            memcpy(_buf + write_pos, data, first_size * sizeof(T));
            memcpy(_buf, data + first_size, second_size * sizeof(T));
            write_pos = second_size;
        }

        _write_pos.store(write_pos, std::memory_order_release);

        return true;
    };

    T *ReadBegin(size_t *size)
    {
        const size_t write_pos = _write_pos.load(std::memory_order_acquire);
        const size_t read_pos = _read_pos.load(std::memory_order_relaxed);

        if (unlikely(read_pos == write_pos))
        {
            *size = 0;
            return nullptr;
        }

        if (read_pos < write_pos)
        {
            *size = write_pos - read_pos;
        }
        else
        {
            *size = TOTAL_SIZE - read_pos;
        }

        return _buf + read_pos;
    };

    void ReadEnd(size_t size)
    {
        size_t read_pos = _read_pos.load(std::memory_order_relaxed) + size;
        read_pos &= MASK;
        _read_pos.store(read_pos, std::memory_order_release);
    };

    T *Read(size_t size)
    {
// LogDebug("%d %d %d %d", _read_pos, _write_pos, _write_pos - _read_pos, ((_write_pos - _read_pos) & (_total_size - 1)) < _block_size);
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
        if ((TOTAL_SIZE / size) * size != TOTAL_SIZE)
        {
            LogError("_total_size must be a multiple of size.");
        }
#endif
        if (unlikely(AvailableSize() < size))
            return nullptr;

        size_t read_pos = _read_pos.load(std::memory_order_relaxed);
        T *buf = _buf + read_pos;

        // LogDebug("read_pos %d %d", read_pos, SIZE);
        read_pos += size;
        if (read_pos >= TOTAL_SIZE)
        {
            read_pos = 0;
        }
        _read_pos.store(read_pos, std::memory_order_release);

        return buf;
    }

    // the size can be written
    size_t FreeSize()
    {
        const size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        const size_t read_pos = _read_pos.load(std::memory_order_acquire);
        return (read_pos - write_pos - 1) & MASK;
    };

    // the size can be read
    size_t AvailableSize()
    {
        const size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        const size_t read_pos = _read_pos.load(std::memory_order_acquire);
        return (write_pos - read_pos) & MASK;
    }

    size_t TotalSize()
    {
        return TOTAL_SIZE;
    }

    size_t OccupancySize()
    {
        return AvailableSize() * 100 / TOTAL_SIZE;
    }

protected:
    T *_GetTmpBuf(size_t size)
    {
        if (unlikely(size > _tmp_size))
        {
            if (_tmp != nullptr)
                delete[] _tmp;
            _tmp_size = size * 2;
            _tmp = new T[_tmp_size];
        }

        return _tmp;
    }

    static constexpr size_t MASK = TOTAL_SIZE - 1;

    T *_buf;
    alignas(32) std::atomic_size_t _read_pos;
    alignas(32) std::atomic_size_t _write_pos;

    T *_tmp;
    size_t _tmp_size;
};