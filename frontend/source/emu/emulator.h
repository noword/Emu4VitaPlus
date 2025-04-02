#pragma once
#include <string>
#include <libretro.h>
#include <stdint.h>
#include <vector>
#include "texture_buf.h"
#include "audio.h"
#include "delay.h"
#include "input.h"
#include "archive_manager.h"
#include "rewind.h"
#include "cheat.h"
#include "arcade_manager.h"
#include "disk_control.h"
#include "rect.h"
#include "keyboard.h"

using namespace Emu4VitaPlus;

#define MAX_SPEED 3.0
#define MIN_SPEED 0.3

enum VIDEO_ROTATION
{
    VIDEO_ROTATION_0 = 0,
    VIDEO_ROTATION_90,
    VIDEO_ROTATION_180,
    VIDEO_ROTATION_270
};

extern bool EnvironmentCallback(unsigned cmd, void *data);
extern void VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch);
extern void AudioSampleCallback(int16_t left, int16_t right);
extern size_t AudioSampleBatchCallback(const int16_t *data, size_t frames);
extern void InputPollCallback();
extern int16_t InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id);

class Emulator
{
public:
    Emulator();
    virtual ~Emulator();

    void Init();
    bool LoadRom(const char *path, const char *entry_name, uint32_t crc32);
    void UnloadGame();
    void Reset();
    void Run();
    void Show();
    void Save();
    void Load();
    int32_t Lock(uint32_t *timeout = NULL);
    void Unlock();
    void SetSpeed(double speed);
    void Wait();
    const char *GetCurrentName() { return _current_name.c_str(); };
    bool SaveScreenShot(const char *name);
    bool GetCurrentSoftwareFramebuffer(retro_framebuffer *fb);
    const char *GetValidExtensions() const { return _info.valid_extensions; };
    const char *GetCoreName() const { return _info.library_name; };
    const char *GetCoreVersion() const { return _info.library_version; };
    double GetSampleRate() const { return _av_info.timing.sample_rate; };
    float GetAspectRatio() { return _av_info.geometry.aspect_ratio; };
    uint64_t GetMsPerFrame() { return _delay.GetInterval(); };
    void ChangeGraphicsConfig() { _graphics_config_changed = true; };
    void ChangeRewindConfig();
    void ChangeAudioConfig();
    uint32_t GetFrameCount() { return _frame_count; };
    void CoreOptionUpdate();
    Cheats *GetCheats() { return &_cheats; };
    void ChangeCheatConfig();
    void SetupKeys();
    void SetupKeysWithSaveConfig();
    bool NeedRender();
    const ArcadeManager *GetArcadeManager() const { return _arcade_manager; };

    template <typename T>
    void SetDiskControlCallback(const T *callback)
    {
        if (_disk_contorl)
        {
            delete _disk_contorl;
            _disk_contorl = nullptr;
        }

        if (callback)
        {
            _disk_contorl = new DiskControl(callback);
        }
    }

    bool IsMultiDisc();
    DiskControl *GetDiskControl() { return _disk_contorl; };

    friend bool EnvironmentCallback(unsigned cmd, void *data);
    friend void VideoRefreshCallback(const void *data, unsigned width, unsigned height, size_t pitch);
    friend void AudioSampleCallback(int16_t left, int16_t right);
    friend size_t AudioSampleBatchCallback(const int16_t *data, size_t frames);
    friend void InputPollCallback();
    friend int16_t InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id);

private:
    void _SetPixelFormat(retro_pixel_format format);
    void _SetVideoSize(uint32_t width, uint32_t height);
    void _SetVertices(float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad);
    void _CreateTextureBuf(SceGxmTextureFormat format, size_t width, size_t height);
    void _SetupVideoOutput(unsigned width, unsigned height);
    void _SetControllerInfo(retro_controller_info *info);

    int16_t _GetJoypadState(unsigned index, unsigned id);
    int16_t _GetAnalogState(unsigned index, unsigned id);
    int16_t _GetMouseState(unsigned index, unsigned id);
    int16_t _GetLightGunState(unsigned index, unsigned id);
    int16_t _GetPointerState(unsigned index, unsigned id);
    int16_t _GetKeybaordState(unsigned index, unsigned id);

    void _OnPsButton(Input *input);
    void _OnHotkeySave(Input *input);
    void _OnHotkeyLoad(Input *input);
    void _OnHotkeySpeedUp(Input *input);
    void _OnHotkeySpeedDown(Input *input);
    void _OnHotkeyRewind(Input *input);
    void _OnHotkeyRewindUp(Input *input);
    // void _OnHotkeyCtrlPortUp(Input *input);
    // void _OnHotkeyCtrlPortDown(Input *input);
    void _OnHotkeyExitGame(Input *input);
    void _OnHotkeyKeyboard(Input *input);
    void _OnHotkeyKeyboardUp(Input *input);
    void _OnHotkeyKeyboardDown(Input *input);

    std::string _SaveDirPath();
    std::string _SaveNamePath(uint32_t id);

    bool _LoadCheats(const char *path);
    void _InitArcadeManager();
    void _ShowSpeedHint();

    std::string _current_name;
    bool _loaded;
    retro_system_info _info;
    retro_system_av_info _av_info;

    SceGxmTextureFormat _video_pixel_format;
    retro_pixel_format _retro_pixel_format;
    TextureBuf *_texture_buf;
    Rect<int> _video_rect;
    bool _graphics_config_changed;
    VIDEO_ROTATION _video_rotation;
    Delay _video_delay;
    vita2d_texture *_last_texture;

    Audio _audio;
    Input _input;
    uint32_t _keys[16];
    uint32_t _keys_mask;
    double _speed;
    Delay _delay;
    uint32_t _frame_count;
    bool _show_video;

    SceKernelLwMutexWork _run_mutex;

    ArchiveManager _archive_manager;
    RewindManager _rewind_manager;
    Cheats _cheats;

    vita2d_texture_vertex _vertices[4];

    retro_core_options_update_display_callback_t _core_options_update_display_callback;
    bool _core_options_updated;

    ArcadeManager *_arcade_manager;

    DiskControl *_disk_contorl;
    Keyboard *_keyboard;
};

extern Emulator *gEmulator;