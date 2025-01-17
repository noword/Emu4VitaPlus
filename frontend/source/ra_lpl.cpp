#include <psp2/io/dirent.h>
#include <zlib.h>
#include <string.h>
#include <jsoncpp/json/json.h>
#include <vita2d.h>
#include <SimpleIni.h>
#include "ra_lpl.h"
#include "file.h"
#include "log.h"
#include "utils.h"
#include "defines.h"

#define RETRO_ARCH_UX0_PATH "ux0:data/retroarch"
#define RETRO_ARCH_UX0_CONFIG RETRO_ARCH_UX0_PATH "/retroarch.cfg"
#define RETRO_ARCH_UX0_PLAYLISTS_PATH RETRO_ARCH_UX0_PATH "/playlists"
#define RETRO_ARCH_UX0_THUMBNAILS_PATH RETRO_ARCH_UX0_PATH "/thumbnails"

#define RETRO_ARCH_UMA0_PATH "uma0:data/retroarch"
#define RETRO_ARCH_UMA0_CONFIG RETRO_ARCH_UMA0_PATH "/retroarch.cfg"
#define RETRO_ARCH_UMA0_PLAYLISTS_PATH RETRO_ARCH_UMA0_PATH "/playlists"
#define RETRO_ARCH_UMA0_THUMBNAILS_PATH RETRO_ARCH_UMA0_PATH "/thumbnails"

#define RETRO_ARCH_BOXARTS "Named_Boxarts"
#define RETRO_ARCH_SNAPS "Named_Snaps"
#define RETRO_ARCH_TITLES "Named_Titles"
#define LPL_EXT "lpl"

RetroArchPlaylists *gPlaylists;

static const std::string THUMBNAILS_PATHS[] = {RETRO_ARCH_BOXARTS, RETRO_ARCH_SNAPS, RETRO_ARCH_TITLES};

RetroArchPlaylists::RetroArchPlaylists()
{
}

RetroArchPlaylists::~RetroArchPlaylists()
{
}

void RetroArchPlaylists::LoadAll()
{
    LogFunctionName;

    Load(PLAYLISTS_DIR);

    CSimpleIniA ini;
    if (ini.LoadFile(RETRO_ARCH_UX0_CONFIG) == SI_OK)
    {
        std::string pl_dir = ini.GetValue("", "playlist_directory", "NULL");
        Utils::StripQuotes(&pl_dir);
        LogDebug(pl_dir.c_str());
        Load(pl_dir.c_str());
    }
    else
    {
        Load(RETRO_ARCH_UX0_PLAYLISTS_PATH);
    }

    if (ini.LoadFile(RETRO_ARCH_UMA0_CONFIG) == SI_OK)
    {
        std::string pl_dir = ini.GetValue("", "playlist_directory", "NULL");
        Utils::StripQuotes(&pl_dir);
        LogDebug(pl_dir.c_str());
        Load(pl_dir.c_str());
    }
    else
    {
        Load(RETRO_ARCH_UMA0_PLAYLISTS_PATH);
    }
}

void RetroArchPlaylists::Load(const char *dir_path)
{
    SceUID dfd = sceIoDopen(dir_path);
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
            std::string path = std::string(dir_path) + "/" + dir.d_name;

            uint32_t crc = _GetLplCrc32(path.c_str());
            ItemMap items;
            if ((!_LoadCache(crc, items)) && _LoadLpl(path.c_str(), items))
            {
                _SaveCache(crc, items);
            }

            if (items.size() > 0)
            {
                LogDebug("%d items loaded", items.size());
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
    LogFunctionName;
    uint32_t crc = crc32(0, (uint8_t *)path, strlen(path));
    auto iter = _items.find(crc);
    if (iter != _items.end())
    {
        std::string *label = &(iter->second.label);
        vita2d_texture *texture;

        for (const auto &root : {THUMBNAILS_DIR, RETRO_ARCH_UX0_THUMBNAILS_PATH, RETRO_ARCH_UMA0_THUMBNAILS_PATH})
        {
            const std::string root_path = std::string(root) + "/" + _dbs[iter->second.db_name_index] + "/";
            for (const auto &path : THUMBNAILS_PATHS)
            {
                std::string im_path = root_path + path + "/" + *label;
                texture = vita2d_load_PNG_file((im_path + ".png").c_str());
                if (texture)
                    return texture;
                texture = vita2d_load_JPEG_file((im_path + ".jpg").c_str());
                if (texture)
                    return texture;
            }
        }
    }
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
            size_t size = strlen(path);
            if (path && label && size > 5)
            {
                uint32_t crc;
                if (path[4] == '/')
                {
                    crc = crc32(0, (uint8_t *)path, size);
                }
                else
                {
                    std::string s = std::string(path, 4) + "/" + (path + 4);
                    crc = crc32(0, (uint8_t *)s.c_str(), s.size());
                }
                items[crc] = {db_index, label};
            }
        }

        result = true;
    }

END:
    delete reader;
    delete[] buf;
    return result;
}

static inline void WriteStr(FILE *fp, const std::string &s)
{
    size_t size = s.size();
    fwrite(&size, sizeof(uint16_t), 1, fp);
    fwrite(s.c_str(), size, 1, fp);
}

static inline std::string ReadStr(FILE *fp)
{
    uint16_t size;
    char s[SCE_FIOS_PATH_MAX];
    fread(&size, sizeof(uint16_t), 1, fp);
    fread(s, size, 1, fp);
    s[size] = '\x00';
    return s;
}

bool RetroArchPlaylists::_LoadCache(uint32_t crc32, ItemMap &items)
{
    char name[SCE_FIOS_PATH_MAX];
    snprintf(name, SCE_FIOS_PATH_MAX, PLAYLISTS_CACHE_DIR "/%08x.bin", crc32);
    FILE *fp = fopen(name, "rb");
    if (!fp)
    {
        LogError("failed to open %s for writing", name);
        return false;
    }

    uint8_t db_index = _GetDbIndex(ReadStr(fp).c_str());
    size_t size;
    fread(&size, sizeof(size_t), 1, fp);
    for (size_t i = 0; i < size; i++)
    {
        uint32_t crc;
        fread(&crc, sizeof(uint32_t), 1, fp);
        items[crc] = {db_index, ReadStr(fp)};
    }

    fclose(fp);

    return true;
}

bool RetroArchPlaylists::_SaveCache(uint32_t crc32, const ItemMap &items)
{
    if (items.size() == 0)
    {
        return false;
    }

    if (!File::Exist(PLAYLISTS_CACHE_DIR))
    {
        File::MakeDirs(PLAYLISTS_CACHE_DIR);
    }

    char name[SCE_FIOS_PATH_MAX];
    snprintf(name, SCE_FIOS_PATH_MAX, PLAYLISTS_CACHE_DIR "/%08x.bin", crc32);
    FILE *fp = fopen(name, "wb");
    if (!fp)
    {
        LogError("failed to open %s for writing", name);
        return false;
    }

    WriteStr(fp, _dbs[items.begin()->second.db_name_index]);
    size_t size = items.size();
    fwrite(&size, sizeof(size_t), 1, fp);

    for (const auto &item : items)
    {
        fwrite(&item.first, sizeof(uint32_t), 1, fp);
        WriteStr(fp, item.second.label);
    }

    fclose(fp);
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