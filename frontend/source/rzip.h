#pragma once
#include <stdint.h>

#define RZIP_MAGIC "#RZIPv"

class Rzip
{
public:
    Rzip(const char *path = nullptr);
    virtual ~Rzip();
    bool Load(const char *path);
    bool Load(uint8_t *rzip_buf);
    bool IsValid() { return _buf != nullptr; };
    const uint8_t *GetBuf() { return _buf; };
    size_t GetSize() { return _size; };

private:
    uint8_t *_buf;
    size_t _size;
};