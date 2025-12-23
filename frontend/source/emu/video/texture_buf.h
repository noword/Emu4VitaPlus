#pragma once
#include <vita2d.h>
#include <array>
#include "log.h"
#include "defines.h"
#include "utils.h"

#define DEFAULT_TEXTURE_BUF_COUNT 16
#if !(DEFAULT_TEXTURE_BUF_COUNT > 0 && (DEFAULT_TEXTURE_BUF_COUNT & (DEFAULT_TEXTURE_BUF_COUNT - 1)) == 0)
#error DEFAULT_TEXTURE_BUF_COUNT must be a power of 2
#endif

#define POWER2_LOOP_PLUS_ONE(VALUE, TOTAL) \
    {                                      \
        VALUE++;                           \
        VALUE &= TOTAL - 1;                \
    }

class TextureBuf : public std::array<vita2d_texture *, DEFAULT_TEXTURE_BUF_COUNT>
{
public:
    TextureBuf(SceGxmTextureFormat format, size_t width, size_t height)
        : _width(width), _height(height), _index(0)
    {
        LogFunctionName;

        LogDebug("  %d x %d (%d)", width, height, format);

        for (auto &texture : *this)
        {
            texture = vita2d_create_empty_texture_format(width, height, format);
        }
    };

    virtual ~TextureBuf()
    {
        LogFunctionName;

        vita2d_wait_rendering_done();
        for (auto &texture : *this)
        {
            vita2d_free_texture(texture);
        }
    }

    size_t GetWidth() const { return _width; };
    size_t GetHeight() const { return _height; };

    vita2d_texture *Next()
    {
        POWER2_LOOP_PLUS_ONE(_index, size());
        return (*this)[_index];
    };

    vita2d_texture *NextBegin()
    {
        size_t index = _index;
        POWER2_LOOP_PLUS_ONE(index, size());
        return (*this)[index];
    };

    void NextEnd()
    {
        POWER2_LOOP_PLUS_ONE(_index, size());
    };

    vita2d_texture *Current()
    {
        return (*this)[_index];
    };

    void SetFilter(SceGxmTextureFilter filter)
    {
        for (auto &texture : *this)
        {
            vita2d_texture_set_filters(texture, filter, filter);
        }
    }

private:
    size_t _width, _height;
    size_t _index;
};