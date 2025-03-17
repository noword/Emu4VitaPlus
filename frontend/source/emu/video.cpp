#include <math.h>
#include <imgui_vita2d/imgui_impl_vita2d.h>
#include <shared.h>
#include "emulator.h"
#include "config.h"
#include "app.h"
#include "overlay.h"
#include "shader.h"
#include "profiler.h"

extern float _vita2d_ortho_matrix[4 * 4];

void VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    LogFunctionNameLimited;

    if (width == 0 || height == 0)
    {
        LogDebug("  invalid size: %d %d", width, height);
        // gEmulator->_delay.Wait();
        return;
    }

    if (gEmulator->_graphics_config_changed ||
        gEmulator->_texture_buf == nullptr ||
        gEmulator->_texture_buf->GetWidth() != width ||
        gEmulator->_texture_buf->GetHeight() != height)
    {
        if (gEmulator->_texture_buf)
        {
            LogDebug("  old: (%d, %d) new: (%d, %d)",
                     gEmulator->_texture_buf->GetWidth(),
                     gEmulator->_texture_buf->GetHeight(),
                     width,
                     height);

            LogDebug("  base width: %d base height: %d aspect ratio: %0.4f",
                     gEmulator->_av_info.geometry.base_width,
                     gEmulator->_av_info.geometry.base_height,
                     gEmulator->_av_info.geometry.aspect_ratio);
        }

        gEmulator->_SetupVideoOutput(width, height);
    }

    if ((!data) || pitch == 0)
    {
        return;
    }

    BeginProfile("VideoRefreshCallback");

    vita2d_texture *texture = gEmulator->_texture_buf->NextBegin();

    if (data != vita2d_texture_get_datap(texture))
    {
        unsigned out_pitch = vita2d_texture_get_stride(texture);
        uint8_t *out = (uint8_t *)vita2d_texture_get_datap(texture);
        uint8_t *in = (uint8_t *)data;

        if (pitch == out_pitch)
        {
            memcpy(out, in, pitch * height);
        }
        else
        {
            unsigned p = std::min(pitch, out_pitch);
            for (unsigned i = 0; i < height; i++)
            {
                memcpy(out, in, p);
                in += pitch;
                out += out_pitch;
            }
        }
    }

    gEmulator->_texture_buf->NextEnd();
    gEmulator->_frame_count++;
    gVideo->Signal();

    if (CONTROL_SPEED_BY_VIDEO)
    {
        gEmulator->Wait();
    }

    EndProfile("VideoRefreshCallback");
}

bool Emulator::NeedRender()
{
    return (!gEmulator->_graphics_config_changed) &&
           gEmulator->_texture_buf != nullptr &&
           _last_texture != _texture_buf->Current();
}

void Emulator::Show()
{
    LogFunctionNameLimited;

    APP_STATUS status = gStatus.Get();

    if (gEmulator->_graphics_config_changed || gEmulator->_texture_buf == nullptr || !(status & (APP_STATUS_RUN_GAME | APP_STATUS_REWIND_GAME | APP_STATUS_SHOW_UI_IN_GAME)))
    {
        sceKernelDelayThread(100000);
        return;
    }

    if (gConfig->graphics[GRAPHICS_OVERLAY] > 0 && gConfig->graphics[GRAPHICS_OVERLAY_MODE] == CONFIG_GRAPHICS_OVERLAY_MODE_BACKGROUND)
    {
        vita2d_texture *tex = (*gOverlays)[gConfig->graphics[GRAPHICS_OVERLAY] - 1].Get();
        if (tex)
        {
            vita2d_draw_texture(tex, 0.f, 0.f);
        }
    }

    // LogDebug("Show _texture_buf->Current() %08x", _texture_buf->Current());

    // vita2d_shader *shader = gConfig->graphics[GRAPHICS_SHADER] > 0 ? (*gShaders)[gConfig->graphics[GRAPHICS_SHADER] - 1].Get() : nullptr;

    uint32_t index = gConfig->graphics[GRAPHICS_SHADER];
    Shader *shader = (index > 0 && index <= gShaders->size()) ? &(*gShaders)[index - 1] : nullptr;

    _last_texture = _texture_buf->Current();
    const SceGxmProgramParameter *wvp_param;

    if (shader && shader->Valid())
    {
        vita2d_shader *_shader = shader->Get();
        vita2d_set_shader(_shader);
        float *texture_size = (float *)vita2d_pool_memalign(4 * sizeof(float), sizeof(float));
        float *output_size = texture_size + 2;
        texture_size[0] = _texture_buf->GetWidth();
        texture_size[1] = _texture_buf->GetHeight();
        output_size[0] = _video_rect.width;
        output_size[1] = _video_rect.height;
        shader->SetUniformData(texture_size, output_size);
        wvp_param = _shader->wvpParam;
    }
    else
    {
        sceGxmSetVertexProgram(vita2d_get_context(), _vita2d_textureVertexProgram);
        sceGxmSetFragmentProgram(vita2d_get_context(), _vita2d_textureFragmentProgram);

        wvp_param = _vita2d_textureWvpParam;
    }

    void *vertex_wvp_buffer;
    sceGxmReserveVertexDefaultUniformBuffer(vita2d_get_context(), &vertex_wvp_buffer);
    sceGxmSetUniformDataF(vertex_wvp_buffer, wvp_param, 0, 16, _vita2d_ortho_matrix);

    vita2d_texture_vertex *vertices = (vita2d_texture_vertex *)vita2d_pool_memalign(4 * sizeof(vita2d_texture_vertex), // 4 vertices
                                                                                    sizeof(vita2d_texture_vertex));
    memcpy(vertices, _vertices, 4 * sizeof(vita2d_texture_vertex));

    sceGxmSetFragmentTexture(vita2d_get_context(), 0, &_last_texture->gxm_tex);
    sceGxmSetVertexStream(vita2d_get_context(), 0, vertices);
    sceGxmDraw(vita2d_get_context(), SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, vita2d_get_linear_indices(), 4);

    // LogDebug("%f %f %f %f", _video_rect.x, _video_rect.y, _video_rect.width, _video_rect.height);
    // LogDebug("%f %f", _video_rect.width / _texture_buf->GetWidth(), _video_rect.height / _texture_buf->GetHeight());

    if (gConfig->graphics[GRAPHICS_OVERLAY] > 0 && gConfig->graphics[GRAPHICS_OVERLAY_MODE] == CONFIG_GRAPHICS_OVERLAY_MODE_OVERLAY)
    {
        vita2d_texture *tex = (*gOverlays)[gConfig->graphics[GRAPHICS_OVERLAY] - 1].Get();
        if (tex)
        {
            vita2d_draw_texture(tex, 0.f, 0.f);
        }
    }

    _video_delay.Wait();
}

bool Emulator::GetCurrentSoftwareFramebuffer(retro_framebuffer *fb)
{
    LogFunctionNameLimited;
    if (!fb || _texture_buf == nullptr || _graphics_config_changed)
    {
        return false;
    }

    // LogDebug("GetCurrentSoftwareFramebuffer _texture_buf->Current() %08x", _texture_buf->Current());

    vita2d_texture *texture = _texture_buf->NextBegin();

    if (fb->width != vita2d_texture_get_width(texture) ||
        fb->height != vita2d_texture_get_height(texture))
    {
        _SetupVideoOutput(fb->width, fb->height);
        texture = _texture_buf->NextBegin();
    }

    fb->data = vita2d_texture_get_datap(texture);
    fb->pitch = vita2d_texture_get_stride(texture);
    fb->format = _retro_pixel_format;
    fb->access_flags = RETRO_MEMORY_ACCESS_WRITE | RETRO_MEMORY_ACCESS_READ;
    fb->memory_flags = RETRO_MEMORY_TYPE_CACHED;

    return true;
}

void Emulator::_SetPixelFormat(retro_pixel_format format)
{
    LogFunctionName;

    _retro_pixel_format = format;
    SceGxmTextureFormat old_format = _video_pixel_format;
    switch (format)
    {
    case RETRO_PIXEL_FORMAT_0RGB1555:
        _video_pixel_format = SCE_GXM_TEXTURE_FORMAT_A1R5G5B5;
        break;

    case RETRO_PIXEL_FORMAT_XRGB8888:
        _video_pixel_format = SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB;
        break;

    case RETRO_PIXEL_FORMAT_RGB565:
        _video_pixel_format = SCE_GXM_TEXTURE_FORMAT_U5U6U5_RGB;
        break;

    default:
        LogWarn("  unknown pixel format: %d", format);
        break;
    }

    LogDebug("  _video_pixel_format: %d", format);
    if (_texture_buf != nullptr && old_format != _video_pixel_format)
    {
        delete _texture_buf;
        _texture_buf = nullptr;
    }
}

void Emulator::_CreateTextureBuf(SceGxmTextureFormat format, size_t width, size_t height)
{
    LogFunctionName;

    if (_texture_buf != nullptr)
    {
        vita2d_wait_rendering_done();
        delete _texture_buf;
    }

    _texture_buf = new TextureBuf(format, width, height);
    _texture_buf->SetFilter(gConfig->graphics[GRAPHICS_SMOOTH] ? SCE_GXM_TEXTURE_FILTER_LINEAR : SCE_GXM_TEXTURE_FILTER_POINT);
    _last_texture = nullptr;
}

void Emulator::_SetVideoSize(uint32_t width, uint32_t height)
{
    LogFunctionName;
    LogDebug(" %d %d %f", _av_info.geometry.base_width, _av_info.geometry.base_height, _av_info.geometry.aspect_ratio);

    if (gConfig->graphics[GRAPHICS_OVERLAY] > 0)
    {
        const Overlay *overlay = &(*gOverlays)[gConfig->graphics[GRAPHICS_OVERLAY] - 1];
        if (overlay->viewport_width > 0 && overlay->viewport_height > 0)
        {
            _video_rect.InitWithDisplacementAndSize(overlay->viewport_x,
                                                    overlay->viewport_y,
                                                    overlay->viewport_width,
                                                    overlay->viewport_height);
            return;
        }
    }

    float aspect_ratio = .0f;

    switch (gConfig->graphics[DISPLAY_RATIO])
    {
    case CONFIG_DISPLAY_RATIO_BY_DEVICE_SCREEN:
        aspect_ratio = (float)VITA_WIDTH / VITA_HEIGHT;
        break;

    case CONFIG_DISPLAY_RATIO_8_7:
        aspect_ratio = 8.f / 7.f;
        break;

    case CONFIG_DISPLAY_RATIO_4_3:
        aspect_ratio = 4.f / 3.f;
        break;

    case CONFIG_DISPLAY_RATIO_3_2:
        aspect_ratio = 3.f / 2.f;
        break;

    case CONFIG_DISPLAY_RATIO_16_9:
        aspect_ratio = 16.f / 9.f;
        break;

    case CONFIG_DISPLAY_RATIO_BY_GAME_RESOLUTION:
    default:
        if (_av_info.geometry.aspect_ratio <= 0.f)
        {
            aspect_ratio = (float)_av_info.geometry.base_width / _av_info.geometry.base_height;
        }
        else
        {
            aspect_ratio = _av_info.geometry.aspect_ratio;
        }

        if (_video_rotation == VIDEO_ROTATION_90)
        {
            aspect_ratio = 1.f / aspect_ratio;
        }
        break;
    }

    switch (gConfig->graphics[DISPLAY_SIZE])
    {
    case CONFIG_DISPLAY_SIZE_2X:
        width *= 2;
        height *= 2;
        break;

    case CONFIG_DISPLAY_SIZE_3X:
        width *= 3;
        height *= 3;
        break;

    case CONFIG_DISPLAY_SIZE_FULL:
        width = VITA_HEIGHT * aspect_ratio;
        if (width > VITA_WIDTH)
        {
            width = VITA_WIDTH;
            height = VITA_WIDTH / aspect_ratio;
        }
        else
        {
            height = VITA_HEIGHT;
        }
        break;

    case CONFIG_DISPLAY_SIZE_1X:
    default:
        break;
    }

    _video_rect.InitWithDisplacementAndSize(0, 0, width, height);

    LogDebug("  width: %d height:%d", width, height);
    LogDebug("%d %d %d %d", _video_rect.top, _video_rect.left, _video_rect.bottom, _video_rect.right);
}

void Emulator::_SetVertices(float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad)
{
    LogFunctionName;
    LogDebug("  %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f", x, y, tex_x, tex_y, tex_w, tex_h, x_scale, y_scale, rad);

    const float w_half = (tex_w * x_scale) / 2.0f;
    const float h_half = (tex_h * y_scale) / 2.0f;

    const float u0 = tex_x / tex_w;
    const float v0 = tex_y / tex_h;
    const float u1 = (tex_x + tex_w) / tex_w;
    const float v1 = (tex_y + tex_h) / tex_h;

    _vertices[0].x = -w_half;
    _vertices[0].y = -h_half;
    _vertices[0].z = +0.5f;
    _vertices[0].u = u0;
    _vertices[0].v = v0;

    _vertices[1].x = w_half;
    _vertices[1].y = -h_half;
    _vertices[1].z = +0.5f;
    _vertices[1].u = u1;
    _vertices[1].v = v0;

    _vertices[2].x = -w_half;
    _vertices[2].y = h_half;
    _vertices[2].z = +0.5f;
    _vertices[2].u = u0;
    _vertices[2].v = v1;

    _vertices[3].x = w_half;
    _vertices[3].y = h_half;
    _vertices[3].z = +0.5f;
    _vertices[3].u = u1;
    _vertices[3].v = v1;

    const float c = cosf(rad);
    const float s = sinf(rad);
    for (int i = 0; i < 4; ++i)
    { // Rotate and translate
        float _x = _vertices[i].x;
        float _y = _vertices[i].y;
        _vertices[i].x = _x * c - _y * s + VITA_WIDTH / 2 - x;
        _vertices[i].y = _x * s + _y * c + VITA_HEIGHT / 2 - y;
    }
}

void Emulator::_SetupVideoOutput(unsigned width, unsigned height)
{
    LogFunctionName;

    gVideo->Lock();

    _CreateTextureBuf(_video_pixel_format, width, height);
    _SetVideoSize(width, height);
    _SetVertices(_video_rect.displacement_x, _video_rect.displacement_y,
                 0, 0,
                 width, height,
                 (float)_video_rect.width / width,
                 (float)_video_rect.height / height,
                 _video_rotation == VIDEO_ROTATION_90 || _video_rotation == VIDEO_ROTATION_180 ? M_PI : 0);

    _graphics_config_changed = false;
    _last_texture = nullptr;

    gVideo->Unlock();

    _input.GetFrontTouch()->InitMapTable(_video_rect);
    _input.GetFrontTouch()->InitMovingScale((float)width / _video_rect.width, (float)height / _video_rect.height);
    _input.GetRearTouch()->InitMovingScale((float)width / _video_rect.width, (float)height / _video_rect.height);
}