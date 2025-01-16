#include <stdint.h>
#include <mz_strm_zlib.h>
#include "log.h"
#include "rzip.h"
#include "file.h"

struct RZIP_HEADER
{
    char header[6]; // #RZIPv
    uint8_t version;
    char pound; // #
    uint32_t chunk_size;
    uint64_t size;
    uint32_t zsize;
};

Rzip::Rzip(const char *path) : _buf(nullptr)
{
    if (path)
        Load(path);
}

Rzip::~Rzip()
{
    if (_buf)
        delete[] _buf;
}

bool Rzip::Load(const char *path)
{
    if (_buf)
    {
        delete[] _buf;
        _buf = nullptr;
    }

    size_t size = File::GetSize(path);
    if (size < sizeof(RZIP_HEADER))
    {
        return false;
    }

    if (!File::ReadFile(path, (void **)&_buf))
    {
        return false;
    }

    RZIP_HEADER *header = (RZIP_HEADER *)_buf;
    _size = header->size;

    // TODO

    return true;
}