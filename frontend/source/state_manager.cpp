#include <libretro.h>
#include "file.h"
#include "state_manager.h"
#include "defines.h"
#include "log.h"
#include "emulator.h"
#include "video.h"

CoreStateManager *gStateManager = nullptr;
vita2d_texture *State::_empty_texture = nullptr;

State::State(const char *slot_name)
    : _slot_name(slot_name),
      _valid(false),
      _texture(nullptr)
{
}

State::~State()
{
    LogFunctionName;
    if (_texture)
    {
        vita2d_free_texture(_texture);
    }
}

void State::Init(const char *game_name)
{
    LogFunctionName;

    char path[SCE_FIOS_PATH_MAX];
    char name[SCE_FIOS_PATH_MAX];
    snprintf(path, SCE_FIOS_PATH_MAX, "%s/%s", CORE_SAVEFILES_DIR, game_name);
    if (!File::Exist(path))
    {
        File::MakeDirs(path);
        return;
    }

    snprintf(name, SCE_FIOS_PATH_MAX, "%s/state_%s.bin", path, _slot_name.c_str());

    _state_path = name;
    _valid = File::Exist(name);
    if (_valid)
    {
        File::GetCreateTime(name, &_create_time);
    }

    snprintf(name, SCE_FIOS_PATH_MAX, "%s/state_%s.jpg", path, _slot_name.c_str());
    _image_path = name;
    if (_texture)
    {
        vita2d_free_texture(_texture);
    }
    _texture = vita2d_load_JPEG_file(name);
}

bool State::Save()
{
    LogFunctionName;

    // backup the path immediately
    std::string state_path = _state_path;
    std::string image_path = _image_path;

    FILE *fp;

    size_t size = retro_serialize_size();
    char *buf = new char[size];

    bool result = retro_serialize(buf, size);
    if (!result)
    {
        LogError("run retro_serialize failed");
        goto END;
    }

    fp = fopen(state_path.c_str(), "wb");
    if (!fp)
    {
        LogError("failed to open file %s for writing", state_path.c_str());
        goto END;
    }

    fwrite(buf, size, 1, fp);
    fclose(fp);

    _valid = true;
    File::GetCreateTime(state_path.c_str(), &_create_time);

END:
    delete[] buf;

    result &= gEmulator->SaveScreenShot(image_path.c_str());

    if (result)
    {
        if (_texture)
        {
            vita2d_free_texture(_texture);
        }
        _texture = vita2d_load_JPEG_file(image_path.c_str());
    }

    return result;
}

bool State::Load()
{
    LogFunctionName;

    char *buf;
    size_t size = File::ReadFile(_state_path.c_str(), (void **)&buf);
    if (size == 0)
    {
        LogError("failed to read file %s", _state_path.c_str());
        return false;
    }

    bool result = retro_unserialize(buf, size);
    if (result)
    {
        LogDebug("retro_unserialize successfully");
    }
    else
    {
        LogWarn("retro_unserialize failed");
    }

    delete[] buf;

    return result;
}

bool State::Remove()
{
    LogFunctionName;
    _valid = false;
    bool result = File::Remove(_state_path.c_str());
    result &= File::Remove(_image_path.c_str());
    if (_texture)
    {
        gVideo->Lock();
        vita2d_free_texture(_texture);
        _texture = nullptr;
        gVideo->Unlock();
    }

    LogDebug("Removed %d", result);
    return result;
}

CoreStateManager::CoreStateManager()
{
    LogFunctionName;
    states[0] = new State("auto");

    for (int i = 1; i < MAX_STATES; i++)
    {
        char buf[3];
        snprintf(buf, 3, "%02d", i);
        states[i] = new State(buf);
    }

    if (State::_empty_texture == nullptr)
    {
        uint32_t height = STATE_SCREENSHOT_HEIGHT;
        uint32_t width = height * gEmulator->GetAspectRatio();
        if (width == 0)
        {
            width = height * 4 / 3;
        }
        State::_empty_texture = vita2d_create_empty_texture(width, height);
        const auto stride = vita2d_texture_get_stride(State::_empty_texture) / 4;
        auto texture_data = (uint32_t *)vita2d_texture_get_datap(State::_empty_texture);
        for (auto y = 0; y < height; ++y)
            for (auto x = 0; x < width; ++x)
                texture_data[y * stride + x] = 0xff00cc00;
    }
}

CoreStateManager::~CoreStateManager()
{
    if (State::_empty_texture != nullptr)
    {
        vita2d_free_texture(State::_empty_texture);
        State::_empty_texture = nullptr;
    }
    for (size_t i = 0; i < MAX_STATES; i++)
    {
        delete states[i];
    }
}

void CoreStateManager::Init(const char *name)
{
    states[0]->Init(name);
    for (int i = 1; i < MAX_STATES; i++)
    {
        states[i]->Init(name);
    }
}

State *CoreStateManager::GetNewest()
{
    State *newest = nullptr;
    time_t newest_time;
    for (auto state : states)
    {
        if (state->Valid())
        {
            if (newest == nullptr)
            {
                newest = state;
                sceRtcGetTime_t(&state->CreateTime(), &newest_time);
            }
            else
            {
                time_t time;
                sceRtcGetTime_t(&state->CreateTime(), &time);
                if (time > newest_time)
                {
                    newest = state;
                    newest_time = time;
                }
            }
        }
    }
    return newest;
}