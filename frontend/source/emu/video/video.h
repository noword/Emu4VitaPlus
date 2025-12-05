#pragma once
#include <stdint.h>
#include <libretro.h>
#include <vita2d.h>
#include "rect.h"
#include "texture_buf.h"
#include "delay.h"

namespace Emu4VitaPlus
{
    enum VIDEO_ROTATION
    {
        VIDEO_ROTATION_0 = 0,
        VIDEO_ROTATION_90,
        VIDEO_ROTATION_180,
        VIDEO_ROTATION_270
    };

    class Video
    {
    public:
        Video();
        virtual ~Video();

        void SetPixelFormat(retro_pixel_format format);
        void Refresh(const void *data, unsigned width, unsigned height, size_t pitch);
        void Show();

    private:
        retro_pixel_format _retro_pixel_format;
        TextureBuf<> *_texture_buf;
        Rect<int> _video_rect;
        bool _graphics_config_changed;
        VIDEO_ROTATION _video_rotation;
        Delay<double> _video_delay;
    };
}