#include <psp2/io/dirent.h>
#include <zlib.h>
#include <string.h>
#include <jsoncpp/json/json.h>
#include "ra_lpl.h"
#include "file.h"
#include "log.h"

#define RETRO_ARCH_PATH "ux0:data/retroarch"
#define RETRO_ARCH_PLAYLISTS_PATH RETRO_ARCH_PATH "/playlists"
#define RETRO_ARCH_THUMBNAILS_PATH RETRO_ARCH_PATH "/thumbnails"
#define RETRO_ARCH_BOXARTS_PATH RETRO_ARCH_THUMBNAILS_PATH "/Named_Boxarts"
#define RETRO_ARCH_SNAPS_PATH RETRO_ARCH_THUMBNAILS_PATH "/Named_Snaps"
#define RETRO_ARCH_TITLES_PATH "/Named_Titles"
#define LPL_EXT "lpl"

RetroArchPlaylists *gPlaylists;

RetroArchPlaylists::RetroArchPlaylists()
{
}

RetroArchPlaylists::~RetroArchPlaylists()
{
}

void RetroArchPlaylists::LoadAll()
{
    LogFunctionName;
    SceUID dfd = sceIoDopen(RETRO_ARCH_PLAYLISTS_PATH);
    if (dfd < 0)
    {
        return;
    }

    SceIoDirent dir;
    while (sceIoDread(dfd, &dir) > 0)
    {
        if (*dir.d_name == '.')
        {
            continue;
        }
        else if (SCE_S_ISREG(dir.d_stat.st_mode) && File::GetExt(dir.d_name) == LPL_EXT)
        {
            std::string path = std::string(RETRO_ARCH_PLAYLISTS_PATH) + "/" + dir.d_name;

            uint32_t crc = _GetLplCrc32(path.c_str());
            ItemMap items;
            if ((!_LoadCache(crc, items)) && _LoadLpl(path.c_str(), items))
            {
                _SaveCache(crc, items);
            }

            if (items.size() > 0)
            {
                _items.insert(items.begin(), items.end());
            }
        }
    }

    sceIoDclose(dfd);
}

const char *RetroArchPlaylists::GetLabel(const char *path)
{
    uint32_t crc = crc32(0, (uint8_t *)path, strlen(path));
    auto iter = _items.find(crc);
    if (iter == _items.end())
    {
        return nullptr;
    }
    else
    {
        return iter->second.label.c_str();
    }
}

vita2d_texture *RetroArchPlaylists::GetPreviewImage(const char *path)
{
    return nullptr;
}

uint32_t RetroArchPlaylists::_GetLplCrc32(const char *path)
{
    char tmp[SCE_FIOS_PATH_MAX];
    time_t time;
    if (!File::GetCreateTime(path, &time))
    {
        time = 0;
    }

    snprintf(tmp, SCE_FIOS_PATH_MAX, "%s%08x%08x", path, File::GetSize(path), time);
    return crc32(0, (uint8_t *)tmp, strlen(tmp));
}

bool RetroArchPlaylists::_LoadLpl(const char *lpl_path, ItemMap &items)
{
    LogFunctionName;

    LogDebug("  load %s", lpl_path);

    char *buf;
    size_t size = File::ReadFile(lpl_path, (void **)&buf);
    if (size == 0)
    {
        return false;
    }

    bool result = false;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    if (!reader->parse(buf, buf + size, &root, nullptr) && !root.isMember("items"))
    {
        LogWarn("failed to loading %s", lpl_path);
        goto END;
    }

    {
        uint8_t db_index = _GetDbIndex(lpl_path);
        Json::Value array = root["items"];
        for (Json::Value::ArrayIndex i = 0; i != array.size(); i++)
        {
            const char *path = array[i]["path"].asCString();
            const char *label = array[i]["label"].asCString();
            if (path && label)
            {
                items[crc32(0, (uint8_t *)path, strlen(path))] = {db_index, label};
            }
        }

        result = true;
    }

END:
    delete reader;
    delete[] buf;
    return result;
}

bool RetroArchPlaylists::_LoadCache(uint32_t crc32, ItemMap &items)
{
    return false;
}

bool RetroArchPlaylists::_SaveCache(uint32_t crc32, const ItemMap &items)
{
    return true;
}

uint8_t RetroArchPlaylists::_GetDbIndex(const char *name)
{
    std::string stem = File::GetStem(name);
    for (size_t i = 0; i < _dbs.size(); i++)
    {
        if (_dbs[i] == stem)
        {
            return i;
        }
    }

    _dbs.push_back(stem);
    return _dbs.size() - 1;
}