#pragma once
#include <vita2d.h>
#include <array>
#include "log.h"
#include "defines.h"
#include "utils.h"
#include "precomp.h"

#define DEFAULT_TEXTURE_BUF_COUNT 5

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

        for (auto &precomp : _precomps)
        {
            if (precomp)
                delete precomp;
        }
    }

    size_t GetWidth() const { return _width; };
    size_t GetHeight() const { return _height; };

    vita2d_texture *Next()
    {
        LOOP_PLUS_ONE(_index, size());
        return (*this)[_index];
    };

    vita2d_texture *NextBegin()
    {
        size_t index = _index;
        LOOP_PLUS_ONE(index, size());
        return (*this)[index];
    };

    void NextEnd()
    {
        LOOP_PLUS_ONE(_index, size());
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

    void SetShader(Shader *shader, const void *index_data, uint32_t index_count)
    {
        size_t count = 0;
        for (auto &precomp : _precomps)
        {
            if (precomp)
                delete precomp;
            precomp = new Precomp(shader, &((*this)[count]->gxm_tex), index_data, index_count);
            precomp->notification.value = 0;
            precomp->notification.address = sceGxmGetNotificationRegion() + count;
            *precomp->notification.address = 0;
            count++;
        }
    }

    Precomp *GetPrecomp()
    {
        return _precomps[_index];
    }

private:
    size_t _width, _height;
    size_t _index;
    std::array<Precomp *, DEFAULT_TEXTURE_BUF_COUNT> _precomps{nullptr};
};