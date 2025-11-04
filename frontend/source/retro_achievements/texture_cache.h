#pragma once
#include <string>
#include <unordered_map>
#include <vita2d.h>

class TextureCache
{
public:
    TextureCache() {};
    virtual ~TextureCache() { Clear(); };

    void Clear();
    vita2d_texture *Get(const std::string &url, const std::string &path = "");
    vita2d_texture *Get(const std::string &url, uint32_t game_id, uint32_t id, bool unlocked = true);

private:
    std::unordered_map<std::string, vita2d_texture *> _cache;
};