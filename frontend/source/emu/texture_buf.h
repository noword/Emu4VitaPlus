#pragma once
#include <vita2d.h>
#include "log.h"
#include "defines.h"
#include "utils.h"

#define DEFAULT_TEXTURE_BUF_COUNT 10

template <size_t SIZE>
class TextureBuf
{
public:
    TextureBuf(SceGxmTextureFormat format, size_t width, size_t height)
        : _width(width), _height(height), _index(0)
    {
        LogFunctionName;

        LogDebug("  %d x %d (%d)", width, height, format);

        for (size_t i = 0; i < SIZE; i++)
        {
            _buf[i] = vita2d_create_empty_texture_format(width, height, format);
        }
    };

    virtual ~TextureBuf()
    {
        LogFunctionName;

        vita2d_wait_rendering_done();
        for (size_t i = 0; i < SIZE; i++)
        {
            vita2d_free_texture(_buf[i]);
        }
    }

    size_t GetWidth() const { return _width; };
    size_t GetHeight() const { return _height; };

    vita2d_texture *Next()
    {
        LOOP_PLUS_ONE(_index, SIZE);
        return _buf[_index];
    };

    vita2d_texture *NextBegin()
    {
        size_t index = _index;
        LOOP_PLUS_ONE(index, SIZE);
        return _buf[index];
    };

    void NextEnd()
    {
        LOOP_PLUS_ONE(_index, SIZE);
    };

    vita2d_texture *Current()
    {
        return _buf[_index];
    };

    void SetFilter(SceGxmTextureFilter filter)
    {
        for (auto &buf : _buf)
        {
            vita2d_texture_set_filters(buf, filter, filter);
        }
    }

private:
    size_t _width, _height;
    size_t _index;
    vita2d_texture *_buf[SIZE];
};