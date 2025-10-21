#pragma once
#include <string>
#include <vita2d.h>

class Achievement
{
public:
    Achievement(uint32_t game_id, uint32_t id, bool unlocked);
    virtual ~Achievement();
    void SetState(bool unlocked) { _unlocked = unlocked; };
    bool GetState() { return _unlocked; };
    vita2d_texture *GetTexture(bool unlocked);
    vita2d_texture *GetTexture();
    const std::string GetImagePath(bool unlocked);

    std::string title;
    std::string description;
    std::string unlock_time;

private:
    bool _unlocked;
    std::string _image_file_name[2];
    vita2d_texture *_texture[2];
};
