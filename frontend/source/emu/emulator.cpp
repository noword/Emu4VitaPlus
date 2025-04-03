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
#include "profiler.h"
#include "core_spec.h"
#include "rzip.h"

extern "C"
{
#include <libswscale/swscale.h>
}

Emulator *gEmulator = nullptr;

Emulator::Emulator()
    : _texture_buf(nullptr),
      _keys{0},
      _info{0},
      _av_info{0},
      _graphics_config_changed(false),
      _frame_count(0),
      _core_options_update_display_callback(nullptr),
      _arcade_manager(nullptr),
      _video_rotation(VIDEO_ROTATION_0),
      _core_options_updated(false),
      _show_video(true),
      _loaded(false),
      _disk_contorl(nullptr),
      _speed(1.0),
      _keyboard(nullptr)
{
    retro_get_system_info(&_info);
    sceKernelCreateLwMutex(&_run_mutex, "run_mutex", 0, 0, NULL);
    _InitArcadeManager();

    if (ENABLE_KEYBOARD)
    {
        _keyboard = new Keyboard();
    }
}

Emulator::~Emulator()
{
    LogFunctionName;

    if (_texture_buf)
    {
        delete _texture_buf;
    }

    if (_arcade_manager)
    {
        delete _arcade_manager;
    }

    if (_keyboard)
    {
        delete _keyboard;
    }

    sceKernelDeleteLwMutex(&_run_mutex);

    retro_deinit();
}

void Emulator::Init()
{
    LogFunctionName;

    retro_set_environment(EnvironmentCallback);
    retro_set_video_refresh(VideoRefreshCallback);
    retro_set_audio_sample(AudioSampleCallback);
    retro_set_audio_sample_batch(AudioSampleBatchCallback);
    retro_set_input_poll(InputPollCallback);
    retro_set_input_state(InputStateCallback);

    retro_init();
}

bool Emulator::LoadRom(const char *path, const char *entry_name, uint32_t crc32)
{
    LogFunctionName;

    char *buf = nullptr;
    bool result = false;
    retro_game_info game_info = {0};

    if (path == nullptr)
    {
        gStatus.Set(APP_STATUS_BOOT);
        result = retro_load_game(NULL);
        goto LOADED;
    }

    LogDebug(" %s %s %08x", path, entry_name, crc32);

    if (!File::Exist(path))
    {
        return false;
    }

    if (gConfig->reboot_when_loading_again && _loaded)
    {
        _current_name = path;
        gStatus.Set(APP_STATUS_REBOOT_WITH_LOADING);
        return true;
    }

    gStatus.Set(APP_STATUS_BOOT);

    _current_name = File::GetStem(path);
    gStateManager->Init(_current_name.c_str());

    _core_options_updated = false;

    const char *_path;
    if (entry_name && *entry_name && crc32)
    {
        _path = _archive_manager.GetCachedPath(crc32, path, entry_name);
    }
    else if (_arcade_manager != nullptr && _arcade_manager->NeedReplace(path))
    {
        _path = _arcade_manager->GetCachedPath(path);
    }
    else
    {
        _path = path;
    }

    if (_path == nullptr)
    {
        goto LOAD_END;
    }

    game_info.path = _path;
    if (_info.need_fullpath)
    {
        LogDebug("  load rom from path %s", _path);
    }
    else
    {
        LogDebug("  load rom from memory");
        game_info.size = File::ReadFile(_path, (void **)&buf);
        if (game_info.size == 0)
        {
            goto LOAD_END;
        }
        game_info.data = buf;
    }

    gConfig->core_options.Load((std::string(CORE_SAVEFILES_DIR) + "/" + gEmulator->GetCurrentName() + "/core.ini").c_str());

    result = retro_load_game(&game_info);

LOADED:
    if (result)
    {
        gStatus.Set(APP_STATUS_RUN_GAME);

        _last_texture = nullptr;
        retro_get_system_av_info(&_av_info);
        SetupKeys();

        SetSpeed(1.0);
        gUi->ClearLogs();
        // _audio.Init(_av_info.timing.sample_rate);
        gConfig->last_rom = path;
        gConfig->Save();

        Load();
        LogDebug("stack free size: %d", sceKernelCheckThreadStack());
        LogDebug("run first frame");
        retro_run();
        LogDebug("first frame end");

        _frame_count = 0;

        if (gConfig->rewind)
        {
            _rewind_manager.Init();
        }

        if (_LoadCheats(path))
        {
            gUi->UpdateCheatOptions();
            _cheats.Start();
        }

        _loaded = true;
    }

LOAD_END:
    LogDebug("  LoadRom result: %d", result);
    if (buf)
    {
        delete[] buf;
    }

    if (!result)
    {
        gUi->NotificationBootFailed();
        _current_name.clear();
    }

    return result;
}

void Emulator::UnloadGame()
{
    LogFunctionName;
    APP_STATUS status = gStatus.Get();
    gStatus.Set(APP_STATUS_SHOW_UI);

    if (status & (APP_STATUS_RUN_GAME | APP_STATUS_SHOW_UI_IN_GAME))
    {
        Lock();
        _rewind_manager.Deinit();
        _cheats.Stop(true);
        if (gConfig->auto_save)
        {
            gStateManager->states[0]->Save();
        }
        Save();
        retro_unload_game();
        Unlock();

        _last_texture = nullptr;
        if (_disk_contorl)
        {
            delete _disk_contorl;
            _disk_contorl = nullptr;
        }

        LogDebug("game unloaded");
    }
}

void Emulator::Run()
{
    LogFunctionNameLimited;

    uint64_t start = sceKernelGetProcessTimeWide();
    _input.Poll();

    switch (gStatus.Get())
    {
    case APP_STATUS_REWIND_GAME:
        if (gConfig->rewind)
            _rewind_manager.Wait();
        break;
    case APP_STATUS_RUN_GAME:
        if (gConfig->rewind)
            _rewind_manager.Signal();
        break;
    default:
        return;
    }

    BeginProfile("retro_run");

    Lock();
    retro_run();
    _audio.NotifyBufStatus();
    Unlock();

    EndProfile("retro_run");

    if (!CONTROL_SPEED_BY_VIDEO)
    {
        Wait();
    }
}

int32_t Emulator::Lock(uint32_t *timeout)
{
    return sceKernelLockLwMutex(&_run_mutex, 1, timeout);
}

void Emulator::Unlock()
{
    sceKernelUnlockLwMutex(&_run_mutex, 1);
}

void Emulator::Reset()
{
    LogFunctionName;
    if (gStatus.Get() & (APP_STATUS_RUN_GAME | APP_STATUS_SHOW_UI_IN_GAME))
    {
        gStatus.Set(APP_STATUS_BOOT);
        retro_reset();
        gStatus.Set(APP_STATUS_RUN_GAME);
    }
}

void Emulator::SetSpeed(double speed)
{
    LogFunctionName;
    LogDebug("  fps: %f", _av_info.timing.fps);
    _speed = speed;
    uint64_t interval = (uint64_t)(1000000.0 / (_av_info.timing.fps * speed));
    _delay.SetInterval(interval);
    _video_delay.SetInterval(interval);
    _audio.Init(_av_info.timing.sample_rate * speed);
}

void Emulator::_ShowSpeedHint()
{
    char tmp[64];
    snprintf(tmp, 64, "%s %.1fX", TEXT(LANG_SPEED), _speed);
    gUi->SetHint(tmp);
}

void Emulator::Wait()
{
    bool in_time = _delay.Wait();

    if (_show_video)
        _show_video = in_time;
    else
        _show_video = true;
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
        src_format = AV_PIX_FMT_BGRA;
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

static void RotateImage(uint8_t *buf, size_t width, size_t height, VIDEO_ROTATION rotation)
{
    size_t size = width * height * 3;
    uint8_t *new_buf = new uint8_t[size];
    switch (rotation)
    {
    case VIDEO_ROTATION_90:
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int src_idx = (i * width + j) * 3;
                int dst_idx = ((width - j - 1) * height + i) * 3;
                memcpy(&new_buf[dst_idx], &buf[src_idx], 3);
            }
        }
        break;
    case VIDEO_ROTATION_180:
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int src_idx = (i * width + j) * 3;
                int dst_idx = ((height - i - 1) * width + (width - j - 1)) * 3;
                memcpy(&new_buf[dst_idx], &buf[src_idx], 3);
            }
        }
        break;
    case VIDEO_ROTATION_270:
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int src_idx = (i * width + j) * 3;
                int dst_idx = (j * height + (height - i - 1)) * 3;
                memcpy(&new_buf[dst_idx], &buf[src_idx], 3);
            }
        }
        break;
    }

    memcpy(buf, new_buf, size);
    delete[] new_buf;
}

bool Emulator::SaveScreenShot(const char *name)
{
    LogFunctionName;

    FILE *fp = fopen(name, "wb");
    if (!fp)
    {
        return false;
    }

    size_t width = _texture_buf->GetWidth();
    size_t height = _texture_buf->GetHeight();
    uint8_t *buf = new uint8_t[width * height * 3];
    ConvertTextureToRGB888(_texture_buf->Current(), buf, width, height);

    if (_video_rotation != VIDEO_ROTATION_0)
    {
        RotateImage(buf, width, height, _video_rotation);
        if (_video_rotation == VIDEO_ROTATION_90 || _video_rotation == VIDEO_ROTATION_270)
        {
            width = _texture_buf->GetHeight();
            height = _texture_buf->GetWidth();
        }
    }

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
    int pitch = 3 * width;
    for (size_t i = 0; i < height; i++)
    {
        jpeg_write_scanlines(&cinfo, (JSAMPARRAY)&p, 1);
        p += pitch;
    }

    jpeg_finish_compress(&cinfo);
    fclose(fp);
    jpeg_destroy_compress(&cinfo);

    delete[] buf;

    return true;
}

std::string Emulator::_SaveDirPath()
{
    return std::string(CORE_SAVEFILES_DIR) + "/" + gEmulator->_current_name;
}

std::string Emulator::_SaveNamePath(uint32_t id)
{
    std::string s = _SaveDirPath() + "/" + gEmulator->_current_name;
    switch (id)
    {
    case RETRO_MEMORY_SAVE_RAM:
        s += ".srm";
        break;
    case RETRO_MEMORY_RTC:
        s += ".rtc";
        break;
    default:
        s += ".unk";
    }
    return s;
}

static const uint32_t RETRO_MEMORY_IDS[] = {RETRO_MEMORY_SAVE_RAM, RETRO_MEMORY_RTC};

void Emulator::Save()
{
    LogFunctionName;

    std::string path = _SaveDirPath();
    if (!File::Exist(path.c_str()))
    {
        File::MakeDirs(path.c_str());
    }

    for (auto id : RETRO_MEMORY_IDS)
    {
        void *data = NULL;
        size_t size = retro_get_memory_size(id);
        LogDebug("  %d %d", id, size);
        if (size > 0)
        {
            data = retro_get_memory_data(id);
        }

        if (data == NULL)
        {
            LogDebug("failed to save: %d", id);
            continue;
        }

        FILE *fp = fopen(_SaveNamePath(id).c_str(), "wb");
        if (fp)
        {
            fwrite(data, size, 1, fp);
            fclose(fp);
            LogDebug("%s saved", _SaveNamePath(id).c_str());
        }
    }
}

void Emulator::Load()
{
    LogFunctionName;

    for (auto id : RETRO_MEMORY_IDS)
    {
        void *data = NULL;
        size_t size = retro_get_memory_size(id);
        if (size > 0)
        {
            data = retro_get_memory_data(id);
        }
        if (data == NULL)
        {
            LogDebug("no need to load: %d", id);
            continue;
        }

        std::string path = _SaveNamePath(id);
        if (File::Exist(path.c_str()))
        {
            Rzip rzip(path.c_str());
            if (rzip.IsValid() && size == rzip.GetSize())
            {
                memcpy(data, rzip.GetBuf(), size);
                LogDebug("rzip %s loaded", path.c_str());
            }
            else if (File::ReadFile(path.c_str(), data, size))
            {
                LogDebug("%s loaded", path.c_str());
            }
            else
            {
                LogDebug("failed to load %s", path.c_str());
            }
        }
    }
}

void Emulator::ChangeRewindConfig()
{
    LogFunctionName;

    if (gStatus.Get() & (APP_STATUS_RUN_GAME | APP_STATUS_SHOW_UI_IN_GAME))
    {
        if (gConfig->rewind)
        {
            _rewind_manager.Init();
        }
        else
        {
            _rewind_manager.Deinit();
        }
    }
}

void Emulator::ChangeAudioConfig()
{
    LogFunctionName;

    if (gConfig->mute)
    {
        _audio.Deinit();
    }
    else
    {
        _audio.Init(gEmulator->_av_info.timing.sample_rate);
    }
}

void Emulator::CoreOptionUpdate()
{
    LogFunctionName;

    if (_core_options_update_display_callback != nullptr)
    {
        _core_options_update_display_callback();
    }

    _core_options_updated = true;
}

bool Emulator::_LoadCheats(const char *path)
{
    LogFunctionName;

    char p[SCE_FIOS_PATH_MAX];
    std::string dir = File::GetDir(path);
    std::string stem = File::GetStem(path);

    snprintf(p, SCE_FIOS_PATH_MAX, "%s/%s.cht", dir.c_str(), stem.c_str());
    LogDebug(p);
    if (_cheats.Load(p))
    {
        return true;
    }

    snprintf(p, SCE_FIOS_PATH_MAX, "%s/%s/%s.cht", dir.c_str(), CHEAT_DIR_NAME, stem.c_str());
    LogDebug(p);
    if (_cheats.Load(p))
    {
        return true;
    }

    snprintf(p, SCE_FIOS_PATH_MAX, "%s/%s.cht", CORE_CHEATS_DIR, stem.c_str());
    LogDebug(p);
    if (_cheats.Load(p))
    {
        return true;
    }

    return false;
}

void Emulator::ChangeCheatConfig()
{
    LogFunctionName;
    _cheats.Signal();
}

bool Emulator::IsMultiDisc()
{
    return _disk_contorl && _disk_contorl->GetNumImages() > 1;
}