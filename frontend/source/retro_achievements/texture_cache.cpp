#include "texture_cache.h"
#include "global.h"
#include "log.h"

void TextureCache::Clear()
{
    LogFunctionName;
    if (!_cache.empty())
    {
        gVideo->Lock();
        for (auto &c : _cache)
        {
            vita2d_free_texture(c.second);
        }
        _cache.clear();
        gVideo->Unlock();
    }
}

vita2d_texture *TextureCache::Get(const std::string &url, const std::string &path)
{
    LogFunctionName;

    const auto iter = _cache.find(url);
    if (iter != _cache.end())
    {
        return iter->second;
    }

    vita2d_texture *texture = nullptr;
    if (path.empty())
    {
        std::string buf;
        if (gNetwork->Fetch(url.c_str(), &buf))
        {
            texture = vita2d_load_PNG_buffer(buf.c_str());
        }
    }
    else
    {
        if (!File::Exist(path.c_str()))
        {
            gNetwork->Download(url.c_str(), path.c_str());
        }

        texture = vita2d_load_PNG_file(path.c_str());
    }

    if (texture)
        _cache[url] = texture;

    return texture;
}

vita2d_texture *TextureCache::Get(const std::string &url, uint32_t game_id, uint32_t id, bool unlocked)
{
    LogFunctionName;

    std::string path = std::string(RETRO_ACHIEVEMENTS_CACHE_DIR "/") +
                       std::to_string(game_id) +
                       "_" +
                       std::to_string(id) +
                       (unlocked ? "" : "_locked") +
                       ".png";

    return Get(url, path);
}