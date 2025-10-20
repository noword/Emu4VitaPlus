#pragma once
#include <string>
#include <vita2d.h>

struct Achievement
{
    Achievement(uint32_t game_id, uint32_t id);
    virtual ~Achievement();
    void SetState(bool unlocked);

    std::string title;
    std::string description;
    vita2d_texture *texture;
    const std::string GetImagePath(bool unlocked);

private:
    std::string _image_file_name[2];
};
