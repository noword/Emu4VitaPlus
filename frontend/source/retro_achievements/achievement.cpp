#include "achievement.h"
#include "defines.h"
#include "global.h"
#include "log.h"

Achievement::Achievement(uint32_t game_id, uint32_t id, bool unlocked) : _unlocked(unlocked)
{
    LogFunctionName;
    _image_file_name[0] = std::to_string(game_id) + "_" + std::to_string(id) + ".png";
    _image_file_name[1] = std::to_string(game_id) + "_" + std::to_string(id) + "_unlocked.png";
    _texture[0] = _texture[1] = nullptr;
}

Achievement::~Achievement()
{
    LogFunctionName;

    gVideo->Lock();
    for (auto texture : _texture)
    {
        if (texture)
            vita2d_free_texture(texture);
    }
    gVideo->Unlock();
}

const std::string Achievement::GetImagePath(bool unlocked)
{
    return std::string(RETRO_ACHIEVEMENTS_CACHE_DIR "/") + _image_file_name[unlocked ? 1 : 0];
}

vita2d_texture *Achievement::GetTexture(bool unlocked)
{
    int index = unlocked ? 0 : 1;
    if (!_texture[index])
        _texture[index] = vita2d_load_PNG_file(GetImagePath(unlocked).c_str());
    return _texture[index];
}

vita2d_texture *Achievement::GetTexture()
{
    return GetTexture(_unlocked);
}