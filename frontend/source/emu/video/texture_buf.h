#pragma once
#include <stdint.h>
#include <vita2d.h>
#include <libretro.h>
#include <string.h>
#include "utils.h"
#include "log.h"

#define DEFAULT_TEXTURE_BUF_COUNT 4

template <size_t BUF_SIZE = DEFAULT_TEXTURE_BUF_COUNT>
class TextureBuf
{
public:
    TextureBuf(unsigned width,
               unsigned height,
               size_t pitch,
               retro_pixel_format format)
        : _width(width),
          _height(height),
          _pitch(pitch),
          _format(format),
          _index(0),
          _texture_index(0)
    {
        _textures[0] = vita2d_create_empty_texture_format(width, height, _GetVitaPixelFormat(format));
        _textures[1] = vita2d_create_empty_texture_format(width, height, _GetVitaPixelFormat(format));
        _texture_datas[0] = (uint8_t *)vita2d_texture_get_datap(_textures[0]);
        _texture_datas[1] = (uint8_t *)vita2d_texture_get_datap(_textures[1]);

        _out_pitch = vita2d_texture_get_stride(_textures[0]);

        if (pitch == 0)
            pitch = _out_pitch;

        size_t block_size = ALIGN_UP_10H(height * pitch);
        uint8_t *p = _last_buf = _buf = new uint8_t[block_size * BUF_SIZE];
        for (size_t i = 0; i < BUF_SIZE; i++)
        {
            _bufs[i] = p;
            p += block_size;
        }
    }

    virtual ~TextureBuf()
    {
        delete[] _buf;
        vita2d_wait_rendering_done();
        vita2d_free_texture(_textures[0]);
        vita2d_free_texture(_textures[1]);
    }

    unsigned GetWidth() { return _width; }
    unsigned GetHeight() { return _height; }
    size_t GetPitch() { return _pitch; }
    retro_pixel_format GetFormat() { return _format; }

    bool NeedRender() { return _last_buf != Current(); };

    uint8_t *NextBegin()
    {
        size_t index = _index;
        LOOP_PLUS_ONE(index, BUF_SIZE);
        return _bufs[index];
    };

    void NextEnd()
    {
        LOOP_PLUS_ONE(_index, BUF_SIZE);
    };

    uint8_t *Current()
    {
        return _bufs[_index];
    };

    vita2d_texture *GetTexture(bool render = true)
    {
        if (render)
        {
            LOOP_PLUS_ONE(_texture_index, 2);
            uint8_t *in = _last_buf = Current();
            if (_pitch == _out_pitch)
            {
                memcpy(_texture_datas[_texture_index], in, _pitch * _height);
            }
            else
            {
                uint8_t *out = _texture_datas[_texture_index];
                unsigned row_length = std::min(_pitch, _out_pitch);
                for (unsigned i = 0; i < _height; i++)
                {
                    memcpy(out, in, row_length);
                    in += _pitch;
                    out += _out_pitch;
                }
            }
        }
        return _textures[_texture_index];
    };

    void SetFilter(SceGxmTextureFilter filter)
    {
        vita2d_texture_set_filters(_textures[0], filter, filter);
        vita2d_texture_set_filters(_textures[1], filter, filter);
    };

private:
    SceGxmTextureFormat _GetVitaPixelFormat(retro_pixel_format format)
    {
        switch (format)
        {
        case RETRO_PIXEL_FORMAT_0RGB1555:
            return SCE_GXM_TEXTURE_FORMAT_X1U5U5U5_1RGB;

        case RETRO_PIXEL_FORMAT_XRGB8888:
            return SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB;

        case RETRO_PIXEL_FORMAT_RGB565:
            return SCE_GXM_TEXTURE_FORMAT_U5U6U5_RGB;

        default:
            LogError("  unknown pixel format: %d", format);
            return (SceGxmTextureFormat)-1;
        }
    }

    unsigned _width;
    unsigned _height;
    size_t _pitch;
    size_t _out_pitch;
    retro_pixel_format _format;

    uint8_t *_texture_datas[2];
    vita2d_texture *_textures[2];
    uint8_t _texture_index;

    uint8_t *_buf;
    uint8_t *_bufs[BUF_SIZE];
    uint8_t *_last_buf;
    size_t _index;
};