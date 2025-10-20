#include "achievement.h"
#include "defines.h"
#include "global.h"

Achievement::Achievement(uint32_t game_id, uint32_t id) : texture(nullptr)
{
    _image_file_name[0] = std::to_string(game_id) + "_" + std::to_string(id) + "_locked.png";
    _image_file_name[1] = std::to_string(game_id) + "_" + std::to_string(id) + "_unlocked.png";
}

Achievement::~Achievement()
{
    gVideo->Lock();

    if (texture)
        vita2d_free_texture(texture);

    gVideo->Unlock();
}

const std::string Achievement::GetImagePath(bool unlocked)
{
    return std::string(RETRO_ACHIEVEMENTS_CACHE_DIR "/") + _image_file_name[unlocked ? 1 : 0];
}

void Achievement::SetState(bool unlocked)
{
    gVideo->Lock();

    if (texture)
        vita2d_free_texture(texture);
    texture = vita2d_load_PNG_file(GetImagePath(unlocked).c_str());

    gVideo->Unlock();
}