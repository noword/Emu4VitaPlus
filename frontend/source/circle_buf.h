#pragma once
#include <atomic>
#include <string.h>
#include "log.h"

template <typename T>
class CircleBuf
{
public:
    CircleBuf(size_t total_size)
        : _total_size(total_size),
          _read_pos(0),
          _write_pos(0),
          _tmp(nullptr),
          _tmp_size(0),
          _continue_write(true)
    {
        _buf = new T[_total_size];
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

    T *WriteBegin(size_t size)
    {
        if (size > FreeSize())
        {
            return nullptr;
        }

        const size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        _continue_write = (write_pos + size) < _total_size;
        return _continue_write ? _buf + write_pos : _GetTmpBuf(size);
    };

    void WriteEnd(size_t size)
    {
        size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        if (_continue_write)
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
        if (size > FreeSize())
        {
            return false;
        }

        size_t write_pos = _write_pos.load(std::memory_order_relaxed);
        if ((write_pos + size) < _total_size)
        {
            memcpy(_buf + write_pos, data, size * sizeof(T));
            write_pos += size;
        }
        else
        {
            size_t first_size = (_total_size - write_pos);
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

        if (read_pos == write_pos)
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
            *size = _total_size - read_pos;
        }

        return _buf + read_pos;
    };

    void ReadEnd(size_t size)
    {
        size_t read_pos = _read_pos.load(std::memory_order_relaxed) + size;
        if (read_pos == _total_size)
        {
            read_pos = 0;
        }
        _read_pos.store(read_pos, std::memory_order_release);
    };

    T *Read(size_t size)
    {
// LogDebug("%d %d %d %d", _read_pos, _write_pos, _write_pos - _read_pos, ((_write_pos - _read_pos) & (_total_size - 1)) < _block_size);
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
        if ((_total_size / size) * size != _total_size)
        {
            LogError("_total_size must be a multiple of size.");
        }
#endif
        if (AvailableSize() < size)
            return nullptr;

        size_t read_pos = _read_pos.load(std::memory_order_relaxed);
        T *buf = _buf + read_pos;

        // LogDebug("read_pos %d %d", read_pos, SIZE);
        read_pos += size;
        if (read_pos >= _total_size)
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
        if (read_pos > write_pos)
        {
            return read_pos - write_pos;
        }
        else
        {
            return (_total_size - (write_pos - read_pos));
        }
    };

    // the size can be read
    size_t AvailableSize()
    {
        return _total_size - FreeSize();
    }

    size_t TotalSize()
    {
        return _total_size;
    }

    size_t OccupancySize()
    {
        return AvailableSize() * 100 / _total_size;
    }

protected:
    T *_GetTmpBuf(size_t size)
    {
        if (size > _tmp_size)
        {
            if (_tmp != nullptr)
                delete[] _tmp;
            _tmp_size = size * 2;
            _tmp = new T[_tmp_size];
        }

        return _tmp;
    }

    T *_buf;
    size_t _total_size;
    std::atomic_size_t _read_pos;
    std::atomic_size_t _write_pos;

    T *_tmp;
    size_t _tmp_size;

    bool _continue_write;
};