#pragma once
#include <stdint.h>

class Rzip
{
public:
    Rzip(const char *path = nullptr);
    virtual ~Rzip();
    bool Load(const char *path);
    bool IsValid() { return _buf != nullptr; };
    const uint8_t *GetBuf() { return _buf; };
    size_t GetSize() { return _size; };

private:
    uint8_t *_buf;
    size_t _size;
};