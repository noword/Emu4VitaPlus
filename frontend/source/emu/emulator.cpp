#include <algorithm>
#include <string>
#include <string.h>
#include <stdint.h>
#include <jpeglib.h>
#include "emulator.h"
#include "callbacks.h"
#include "app.h"
#include "defines.h"
#include "core_options.h"
#include "state_manager.h"
#include "config.h"
#include "ui.h"
#include "log.h"
#include "file.h"

extern "C"
{
#include <libswscale/swscale.h>
}

Emulator *gEmulator = nullptr;

Emulator::Emulator()
    : _texture_buf(nullptr),
      _keys{0},
      _soft_frame_buf_render(false),
      _current_tex(nullptr),
      _info{0},
      _av_info{0},
      _graphics_config_changed(false)
{
    sceKernelCreateLwMutex(&_run_mutex, "run_mutex", 0, 0, NULL);
    _video_semaid = sceKernelCreateSema("video_sema", 0, 0, 1, NULL);
}

Emulator::~Emulator()
{
    LogFunctionName;

    if (_texture_buf)
    {
        delete _texture_buf;
    }

    sceKernelDeleteLwMutex(&_run_mutex);
    sceKernelDeleteSema(_video_semaid);

    retro_deinit();
}

void Emulator::Init()
{
    LogFunctionName;

    retro_set_environment(EnvironmentCallback);
    retro_set_video_refresh(VideoRefreshCallback);
    retro_set_audio_sample_batch(AudioSampleBatchCallback);
    retro_set_input_poll(InputPollCallback);
    retro_set_input_state(InputStateCallback);

    retro_init();
    retro_get_system_info(&_info);

    retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    _SetupKeys();
}

bool Emulator::LoadGame(const char *path, const char *entry_name, uint32_t crc32)
{
    LogFunctionName;

    gStatus = APP_STATUS_BOOT;

    _current_name = File::GetStem(path);
    gStateManager->Init(_current_name.c_str());

    retro_game_info game_info = {0};
    _soft_frame_buf_render = false;
    char *buf = nullptr;

    if (*entry_name)
    {
        path = _archive_manager.GetCachedPath(crc32, path, entry_name);
    }

    if (_info.need_fullpath)
    {
        LogDebug("  load rom from path");
        game_info.path = path;
    }
    else
    {
        LogDebug("  load rom from memory");
        game_info.size = File::GetSize(path);
        buf = new char[game_info.size];
        if (!File::ReadFile(path, buf, game_info.size))
        {
            LogError("failed to read rom: %s", path);
            delete[] buf;
            return false;
        }
        game_info.data = buf;
    }

    bool result = retro_load_game(&game_info);
    if (result)
    {
        retro_get_system_av_info(&_av_info);
        SetSpeed(1.0);
        gStatus = APP_STATUS_RUN_GAME;
        if (!_audio.Inited())
        {
            _audio.SetSampleRate(_av_info.timing.sample_rate);
        }
    }
    else
    {
        LogError("load rom failed: %s", _current_name.c_str());
        gStatus = APP_STATUS_SHOW_UI;
    }

    gUi->ClearLogs();
    if (buf)
    {
        delete[] buf;
    }

    return result;
}

void Emulator::UnloadGame()
{
    LogFunctionName;
    Lock();
    gStateManager->states[0]->Save();
    retro_unload_game();
    gStatus = APP_STATUS_SHOW_UI;
    Unlock();
}

void Emulator::Run()
{
    _delay.Wait();
    _input.Poll();

    Lock();
    retro_run();
    Unlock();

    if (_soft_frame_buf_render && _texture_buf)
    {
        _texture_buf->Unlock();
        sceKernelSignalSema(_video_semaid, 1);
    }
}

void Emulator::Lock()
{
    sceKernelLockLwMutex(&_run_mutex, 1, NULL);
}

void Emulator::Unlock()
{
    sceKernelUnlockLwMutex(&_run_mutex, 1);
}

void Emulator::Reset()
{
    LogFunctionName;
    retro_reset();
}

void Emulator::SetSpeed(double speed)
{
    LogFunctionName;
    LogDebug("  fps: %f", _av_info.timing.fps);
    _speed = speed;
    _delay.SetInterval(1000000ull / (uint64_t)(_av_info.timing.fps * speed));
}

static void ConvertTextureToRGB888(vita2d_texture *texture, uint8_t *dst, size_t width = 0, size_t height = 0)
{
    AVPixelFormat src_format = AV_PIX_FMT_NONE;
    SceGxmTextureFormat tex_format = vita2d_texture_get_format(texture);
    size_t src_width = vita2d_texture_get_width(texture);
    size_t src_height = vita2d_texture_get_height(texture);
    int src_stride = vita2d_texture_get_stride(texture);
    uint8_t *src = (uint8_t *)vita2d_texture_get_datap(texture);

    if (width == 0)
    {
        width = src_width;
    }

    if (height == 0)
    {
        height = src_height;
    }

    int dst_stride = width * 3;
    LogDebug("  texture format: %d", tex_format);
    switch (tex_format)
    {
    case SCE_GXM_TEXTURE_FORMAT_X1U5U5U5_1RGB:
        src_format = AV_PIX_FMT_RGB555LE;
        break;

    case SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB:
        src_format = AV_PIX_FMT_ARGB;
        break;

    case SCE_GXM_TEXTURE_FORMAT_U5U6U5_RGB:
        src_format = AV_PIX_FMT_RGB565LE;
        break;

    default:
        LogError("  unknown sce gxm format: %d", tex_format);
        return;
    }

    SwsContext *sws_ctx = sws_getContext(src_width, src_height, src_format,
                                         width, height, AV_PIX_FMT_RGB24,
                                         SWS_BILINEAR, NULL, NULL, NULL);
    sws_scale(sws_ctx, &src, &src_stride, 0, src_height, &dst, &dst_stride);
    sws_freeContext(sws_ctx);
}

bool Emulator::SaveScreenShot(const char *name, size_t height)
{
    LogFunctionName;

    FILE *fp = fopen(name, "wb");
    if (!fp)
    {
        return false;
    }

    if (height == 0)
    {
        height = vita2d_texture_get_height(_texture_buf->Current());
    }

    float scale = float(height) / _texture_buf->GetHeight();
    size_t width = _texture_buf->GetWidth() * scale;
    uint8_t *buf = new uint8_t[width * height * 3];
    ConvertTextureToRGB888(_texture_buf->Current(), buf, width, height);
    int dst_stride = 3 * width;

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.in_color_space = JCS_RGB;
    cinfo.input_components = 3;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 90, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    uint8_t *p = buf;
    for (size_t i = 0; i < height; i++)
    {
        jpeg_write_scanlines(&cinfo, (JSAMPARRAY)&p, 1);
        p += dst_stride;
    }

    jpeg_finish_compress(&cinfo);
    fclose(fp);
    jpeg_destroy_compress(&cinfo);

    delete[] buf;

    return true;
}
