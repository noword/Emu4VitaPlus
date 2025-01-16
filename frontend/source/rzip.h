#pragma once
#include <stdint.h>

class Rzip
{
public:
    Rzip(const char *path = nullptr);
    virtual ~Rzip();
    bool Load(const char *path);
    bool IsVaild() { return _buf != nullptr; };
    const char GetBuf();
    size_t GetSize() { return _size; };

private:
    char *_buf;
    size_t _size;
};