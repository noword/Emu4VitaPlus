#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "log.h"
#include "rzip.h"
#include "file.h"

#pragma pack(push, 1)
struct RZIP_HEADER
{
    char sign[6]; // #RZIPv
    uint8_t version;
    char pound; // #
    uint32_t chunk_size;
    uint64_t size;
    uint32_t zsize;
};
#pragma pack(pop)

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

    uint8_t *rzip;
    if (!File::ReadFile(path, (void **)&rzip))
    {
        return false;
    }

    bool result = Load(rzip);
    delete[] rzip;
    return result;
}

bool Rzip::Load(uint8_t *rzip_buf)
{
    bool result = false;

    z_stream infstream{0};
    RZIP_HEADER *header = (RZIP_HEADER *)rzip_buf;
    if (memcmp(header->sign, RZIP_MAGIC, sizeof(header->sign)) != 0)
    {
        goto END;
    }

    _size = header->size;
    _buf = new uint8_t[_size];
    infstream.avail_in = header->zsize;
    infstream.avail_out = _size;
    infstream.next_in = rzip_buf + sizeof(RZIP_HEADER);
    infstream.next_out = _buf;

    if (inflateInit(&infstream) != Z_OK)
        goto END;

    result = inflate(&infstream, Z_NO_FLUSH) >= 0;

    inflateEnd(&infstream);

    // const char *zbuf = _buf + sizeof(RZIP_HEADER);

    result = true;

END:
    if (!result)
    {
        _size = 0;
        if (_buf)
        {
            delete[] _buf;
            _buf = nullptr;
        }
    }

    return result;
}